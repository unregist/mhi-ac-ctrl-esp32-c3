import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import climate

AUTO_LOAD = ["sensor", "climate"]
CODEOWNERS = ["hberntsen"]

CONF_MHI_AC_CTRL_ID = "mhi_ac_ctrl_id"

MhiAcCtrl = cg.global_ns.class_("MhiAcCtrl", cg.Component, climate.Climate)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(MhiAcCtrl),
    }
).extend(cv.COMPONENT_SCHEMA)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
