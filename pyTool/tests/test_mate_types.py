from DataProject import DataProject
from DataEleSub import DataEleSub


def test_addMateType_stores_name_params_defaults():
    p = DataProject("p", 2)
    p.addMateType("HelQ4g", ["ek", "pe"], defaults=[1.0, 2.0])
    assert p.mateTypes == [{"name": "HelQ4g", "params": ["ek", "pe"], "defaults": [1.0, 2.0]}]


def test_addMateType_defaults_empty_when_none():
    p = DataProject("p", 2)
    p.addMateType("M", ["a"])
    assert p.mateTypes[0]["defaults"] == []


def test_DataProject_toDict_fromDict_roundtrip_mateTypes():
    p = DataProject("p", 2)
    p.addMateType("HelQ4g", ["ek", "pe"], defaults=[1.0, 2.0])
    d = p.toDict()
    assert d["mateTypes"] == [{"name": "HelQ4g", "params": ["ek", "pe"], "defaults": [1.0, 2.0]}]
    p2 = DataProject.fromDict(d)
    assert p2.mateTypes == p.mateTypes


def test_DataEleSub_mateTypeName_default_and_roundtrip():
    e = DataEleSub("E", 4)
    assert e.mateTypeName == ""
    e.mateTypeName = "HelQ4g"
    d = e.toDict()
    assert d["mateTypeName"] == "HelQ4g"
    e2 = DataEleSub.fromDict(d)
    assert e2.mateTypeName == "HelQ4g"


def test_DataEleSub_fromDict_legacy_no_mateTypeName():
    legacy = {"name": "E", "nNodes": 4, "paramNames": ["a"]}
    e = DataEleSub.fromDict(legacy)
    assert e.mateTypeName == ""

import jinja2
import os

TEMPLATE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "template"))


def _render(tpl, ctx):
    env = jinja2.Environment(loader=jinja2.FileSystemLoader(TEMPLATE_DIR),
                             trim_blocks=True, lstrip_blocks=True)
    return env.get_template(tpl).render(**ctx)


def test_elesub_template_renders_mateTypeName():
    ele = {"name": "HeatQ4g", "gidName": "HelQ4g", "mateTypeName": "HelQ4g",
           "dispNames": ["T"], "paramNames": ["ek"], "initCode": "",
           "vtkCellType": None, "nNodes": 4, "dim": 2,
           "gaussPoints": [[0,0]], "gaussWeights": [1.0]}
    ctx = {"ele": ele, "project": type("P", (), {"coordVars": ["x", "y"]})(),
           "femDataClassName": "hel2dData", "field": type("F", (), {"fieldDataClassName": "HeatFieldData"})(),
           "baseClass": "IsoEleBase", "headerGuard": "HEATQ4G_H",
           "baseClassParam": "4, pData", "dim": 2, "shapeFuns": []}
    out = _render("elesub.cpp.j2", ctx)
    assert '_mateTypeName = "HelQ4g";' in out
    assert "_paramNames" not in out


def test_domaindata_template_renders_mateConstitutive():
    project = {"name": "hel2d", "dim": 2,
               "fields": [{"fieldDataClassName": "HeatFieldData"}],
               "mateTypes": [{"name": "HelQ4g", "params": ["ek", "pe"], "defaults": []}]}
    ctx = {"femDataClassName": "hel2dData", "project": project}
    out = _render("domaindata.cpp.j2", ctx)
    assert '_mateConstitutive["HelQ4g"] = { "ek", "pe" };' in out

from MakerCpp import MakerCpp


def test_applyMateTypeCompat_defaults_to_ele_name_and_registers():
    p = DataProject("p", 2)
    f = p.addField("F")
    e = DataEleSub("Truss", 2)
    e.paramNames = ["E", "A"]
    e.paramValues = [1.0, 2.0]
    f.addEleSub(e)
    MakerCpp._applyMateTypeCompat(p)
    assert e.mateTypeName == "Truss"
    assert p.mateTypes == [{"name": "Truss", "params": ["E", "A"], "defaults": [1.0, 2.0]}]


def test_applyMateTypeCompat_skips_existing_mateTypeName():
    p = DataProject("p", 2)
    p.addMateType("HelQ4g", ["ek"], [])
    f = p.addField("F")
    e = DataEleSub("HeatQ4g", 4)
    e.mateTypeName = "HelQ4g"
    e.paramNames = ["ek"]
    f.addEleSub(e)
    MakerCpp._applyMateTypeCompat(p)
    assert e.mateTypeName == "HelQ4g"
    assert len(p.mateTypes) == 1  # 不重复注册


def test_applyMateTypeCompat_no_duplicate_for_shared_ele_name():
    p = DataProject("p", 2)
    f = p.addField("F")
    e1 = DataEleSub("El", 4); e1.paramNames = ["pe"]
    e2 = DataEleSub("El", 4); e2.paramNames = ["pe"]
    f.addEleSub(e1); f.addEleSub(e2)
    MakerCpp._applyMateTypeCompat(p)
    assert len(p.mateTypes) == 1
