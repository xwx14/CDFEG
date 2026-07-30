from DataProject import DataProject
from DataField import DataField
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
    ele = {"name": "HeatQ4g", "matName": "HelQ4g", "mateTypeName": "HelQ4g",
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

def test_makeData_singleField_uses_ele_name():
    p = DataProject("p", 1)
    f = p.addField("F")
    e = DataEleSub("Truss1D", 2); e.paramNames = ["E", "A"]; e.paramValues = [1.0, 2.0]
    f.addEleSub(e)
    p.makeData()
    assert e.mateTypeName == "Truss1D"
    assert p.mateTypes == [{"name": "Truss1D", "params": ["E", "A"], "defaults": [1.0, 2.0]}]


def test_makeData_multiField_uses_matName_and_merges_dedup():
    p = DataProject("p", 2)
    f1 = p.addField("Heat"); f2 = p.addField("Del")
    e1 = DataEleSub("HeatQ4g", 4); e1.matName = "HelQ4g"; e1.paramNames = ["ek", "ec", "q"]
    e2 = DataEleSub("DelQ4g", 4); e2.matName = "HelQ4g"; e2.paramNames = ["pe", "pv", "ek"]
    f1.addEleSub(e1); f2.addEleSub(e2)
    p.makeData()
    assert e1.mateTypeName == "HelQ4g" and e2.mateTypeName == "HelQ4g"
    assert len(p.mateTypes) == 1
    assert p.mateTypes[0]["name"] == "HelQ4g"
    assert p.mateTypes[0]["params"] == ["ek", "ec", "q", "pe", "pv"]  # ek 保序去重


def test_makeData_singleField_multi_ele_separate_constitutives():
    p = DataProject("p", 2)
    f = p.addField("F")
    e1 = DataEleSub("ElQ4g", 4); e1.paramNames = ["pe"]
    e2 = DataEleSub("StressBL2g", 2); e2.paramNames = ["fu", "fv"]
    f.addEleSub(e1); f.addEleSub(e2)
    p.makeData()
    assert {mt["name"] for mt in p.mateTypes} == {"ElQ4g", "StressBL2g"}
    assert e1.mateTypeName == "ElQ4g" and e2.mateTypeName == "StressBL2g"
