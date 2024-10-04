#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include "TrainEngine.hpp"

namespace godot {
    class TrainController;

    TrainEngine::TrainEngine() = default;

    void TrainEngine::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_motor_param_table"), &TrainEngine::set_motor_param_table);
        ClassDB::bind_method(D_METHOD("get_motor_param_table"), &TrainEngine::get_motor_param_table);
        ClassDB::bind_method(D_METHOD("_on_command_main_switch", "p1", "p2"), &TrainEngine::_on_command_main_switch);
        ADD_PROPERTY(
                PropertyInfo(
                        Variant::ARRAY, "motor_param_table", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT,
                        "TypedArray<Array>"),
                "set_motor_param_table", "get_motor_param_table");
        ADD_SIGNAL(MethodInfo("engine_start"));
        ADD_SIGNAL(MethodInfo("engine_stop"));
    }

    void TrainEngine::_do_update_internal_mover(TMoverParameters *mover) {
        mover->EngineType = get_engine_type();

        /* FIXME: for testing purposes */
        mover->GroundRelay = true;
        mover->NoVoltRelay = true;
        mover->OvervoltageRelay = true;
        mover->DamageFlag = false;
        mover->EngDmgFlag = false;
        mover->ConvOvldFlag = false;
        /* end testing */

        /* motor param table */
        const int _max = Maszyna::MotorParametersArraySize;
        for (int i = 0; i < std::min(_max, (int)motor_param_table.size()); i++) {
            mover->MotorParam[i].mIsat = motor_param_table[i].get("misat");
            mover->MotorParam[i].fi = motor_param_table[i].get("fi");
            mover->MotorParam[i].mfi = motor_param_table[i].get("mfi");
            mover->MotorParam[i].Isat = motor_param_table[i].get("isat");
            mover->MPTRelay[i].Iup = motor_param_table[i].get("iup");
            mover->MPTRelay[i].Idown = motor_param_table[i].get("idown");
        }
    }

    void TrainEngine::_do_process_mover(TMoverParameters *mover, const double delta) {}

    void TrainEngine::_do_fetch_state_from_mover(TMoverParameters *mover, Dictionary &state) {
        state["Mm"] = mover->Mm;
        state["Mw"] = mover->Mw;
        state["Fw"] = mover->Fw;
        state["Ft"] = mover->Ft;
        state["Im"] = mover->Im;
        state["compressor_enabled"] = mover->CompressorFlag;
        state["compressor_allowed"] = mover->CompressorAllow;
        state["engine_power"] = mover->EnginePower;
        state["engine_rpm_count"] = mover->enrot;
        state["engine_rpm_ratio"] = mover->EngineRPMRatio();
        state["engine_current"] = mover->Im;
        state["engine_damage"] = mover->EngDmgFlag;
        state["main_switch_time"] = mover->MainsInitTimeCountdown;
        state["main_no_power_pos"] = mover->IsMainCtrlNoPowerPos();
        state["camshaft_available"] = mover->HasCamshaft;
        state["converter_overload"] = mover->ConvOvldFlag;
        state["line_breaker_delay"] = mover->CtrlDelay;
        state["line_breaker_initial_delay"] = mover->InitialCtrlDelay;
        state["line_breaker_closes_at_no_power"] = mover->LineBreakerClosesOnlyAtNoPowerPos;
    }

    TypedArray<Dictionary> TrainEngine::get_motor_param_table() {
        return motor_param_table;
    }

    void TrainEngine::set_motor_param_table(const TypedArray<Dictionary> p_value) {
        motor_param_table.clear();
        motor_param_table.append_array(p_value);
    }

    void TrainEngine::_on_command_main_switch(const Variant &p1, const Variant &p2) {
        TMoverParameters *mover = get_mover();
        ASSERT_MOVER(mover);
        mover->MainSwitch((bool)p1);
    }

    void TrainEngine::_bind_commands() {
        bind_command("main_switch", Callable(this, "_on_command_main_switch"));
    }

    void TrainEngine::_unbind_commands() {
        unbind_command("main_switch", Callable(this, "_on_command_main_switch"));
    }

} // namespace godot
