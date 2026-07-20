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
