#include "../core/TrainController.hpp"
#include "TrainSystem.hpp"

namespace godot {
    void TrainSystem::_bind_methods() {
        ClassDB::bind_method(D_METHOD("register_train", "train_id", "train"), &TrainSystem::register_train);
        ClassDB::bind_method(D_METHOD("unregister_train", "train_id"), &TrainSystem::unregister_train);
        ClassDB::bind_method(
                D_METHOD("send_command", "train_id", "command", "p1", "p2"), &TrainSystem::send_command,
                DEFVAL(Variant()), DEFVAL(Variant()));
        ClassDB::bind_method(
                D_METHOD("broadcast_command", "command", "p1", "p2"), &TrainSystem::broadcast_command,
                DEFVAL(Variant()), DEFVAL(Variant()));
        ClassDB::bind_method(D_METHOD("is_command_supported", "command"), &TrainSystem::is_command_supported);
        ClassDB::bind_method(D_METHOD("get_supported_commands"), &TrainSystem::get_supported_commands);
        ClassDB::bind_method(D_METHOD("get_registered_trains"), &TrainSystem::get_registered_trains);
        ClassDB::bind_method(D_METHOD("bind_command", "train_id", "command", "callable"), &TrainSystem::bind_command);
        ClassDB::bind_method(
                D_METHOD("unbind_command", "train_id", "command", "callable"), &TrainSystem::unbind_command);
        ClassDB::bind_method(D_METHOD("get_train_state", "train_id"), &TrainSystem::get_train_state);
        ClassDB::bind_method(D_METHOD("log", "train_id", "loglevel", "line"), &TrainSystem::log);
        ADD_SIGNAL(MethodInfo(
                "train_log_updated", PropertyInfo(Variant::STRING, "train"), PropertyInfo(Variant::INT, "loglevel"),
                PropertyInfo(Variant::STRING, "line")));
        BIND_ENUM_CONSTANT(TRAINLOGLEVEL_DEBUG);
        BIND_ENUM_CONSTANT(TRAINLOGLEVEL_INFO);
        BIND_ENUM_CONSTANT(TRAINLOGLEVEL_WARNING);
        BIND_ENUM_CONSTANT(TRAINLOGLEVEL_ERROR);
    }

    TrainSystem::TrainSystem() {}

    Dictionary TrainSystem::get_train_state(const String &train_id) {
        auto it = trains.find(train_id);

        if (it == trains.end()) {
            log(train_id, TrainLogLevel::TRAINLOGLEVEL_ERROR, "Train is not registered in TrainSystem");
            UtilityFunctions::push_error("Train is not registered in TrainSystem: ", train_id);
            Dictionary empty;
            return empty;
        }
        TrainController *train = it->second;
        return train->get_state();
    }

    void TrainSystem::log(const String &train_id, TrainLogLevel level, const String &line) {
        emit_signal("train_log_updated", train_id, level, line);
    }

    void TrainSystem::register_train(const String &train_id, TrainController *train) {
        trains[train_id] = train;
    }

    void TrainSystem::bind_command(const String &train_id, const String &command, const Callable &callback) {
        auto it = trains.find(train_id);

        if (it == trains.end()) {
            log(train_id, TrainLogLevel::TRAINLOGLEVEL_ERROR, "Train is not registered in TrainSystem");
            UtilityFunctions::push_error("Train is not registered in TrainSystem: ", train_id);
            return;
        }

        TrainController *train = it->second;
        auto instance_id = train->get_instance_id();

        if (!commands.has(command)) {
            Dictionary trains;
            commands[command] = trains;
        }


        if (!((Dictionary)commands[command]).has(instance_id)) {
            Array new_array;
            Dictionary _trains = commands[command];
            _trains[instance_id] = new_array;
        }

        Dictionary _trains = commands[command];
        Array _callbacks = (Array)_trains[instance_id];
        _callbacks.push_back(callback);
    }

    void TrainSystem::unbind_command(const String &train_id, const String &command, const Callable &callback) {
        auto it = trains.find(train_id);

        if (it == trains.end()) {
            log(train_id, TrainLogLevel::TRAINLOGLEVEL_ERROR, "Train is not registered in TrainSystem");
            UtilityFunctions::push_error("Train is not registered in TrainSystem: ", train_id);
            return;
        }
        TrainController *train = it->second;

        if (commands.has(command)) {
            auto instance_id = train->get_instance_id();

            Dictionary _trains = (Dictionary)commands[command];
            Array _callbacks = (Array)_trains[instance_id];
            while (true) {
                int pos = _callbacks.find(callback);
                if (pos < 0) {
                    break;
                }
                _callbacks.remove_at(pos);
            }
            if (_callbacks.size() == 0) {
                _trains.erase(instance_id);
                if (_trains.size() == 0) {
                    commands.erase(command);
                }
            }
        }
    }

    void TrainSystem::unregister_train(const String &train_id) {
        auto it = trains.find(train_id);

        if (it == trains.end()) {
            log(train_id, TrainLogLevel::TRAINLOGLEVEL_ERROR, "Train is not registered in TrainSystem");
            UtilityFunctions::push_error("Train is not registered in TrainSystem: ", train_id);
            return;
        }
        TrainController *train = it->second;

        Array command_keys = (Array)commands.keys();
        auto instance_id = train->get_instance_id();

        Array commands_to_remove;

        for (int i = 0; i < command_keys.size(); i++) {
            String command = command_keys[i];
            Dictionary _trains = commands[command];
            if (_trains.has(instance_id)) {
                _trains.erase(instance_id);
            }
            if (_trains.size() == 0) {
                commands_to_remove.append(command);
            }
        }

        for (int i = 0; i < commands_to_remove.size(); i++) {
            commands.erase(commands_to_remove[i]);
        }

        trains.erase(train_id);
    }

    bool TrainSystem::is_command_supported(const String &command) {
        return commands.has(command);
    }

    Array TrainSystem::get_supported_commands() {
        return commands.keys();
    }

    Array TrainSystem::get_registered_trains() {
        Array train_names;
        for (const auto &pair: trains) {
            train_names.append(pair.first);
        }
        return train_names;
    }

    void
    TrainSystem::send_command(const String &train_id, const String &command, const Variant &p1, const Variant &p2) {
        auto it = trains.find(train_id);

        if (it == trains.end()) {
            log(train_id, TrainLogLevel::TRAINLOGLEVEL_ERROR, "Train is not registered in TrainSystem");
            UtilityFunctions::push_error("Train is not registered in TrainSystem: ", train_id);
            return;
        }
        TrainController *train = it->second;
        if (is_command_supported(command)) {

            Dictionary _trains = (Dictionary)commands[command];
            auto instance_id = train->get_instance_id();

            if (!_trains.has(instance_id)) {
                return;
            }

            Array _callables = _trains[instance_id];

            if (!_callables.size()) {
                UtilityFunctions::push_warning("Command ", command, " has no callables registered");
            }
            int called = 0;
            for (int i = 0; i < _callables.size(); i++) {
                Callable c = _callables[i];
                if (c.is_valid()) {
                    Array args;
                    int arg_required = 0;
                    if (p1.get_type() != Variant::NIL) {
                        arg_required++;
                    }
                    if (p2.get_type() != Variant::NIL) {
                        arg_required++;
                    }
                    int argc = c.get_argument_count();
                    if (argc > 0) {
                        args.append(p1);
                    }
                    if (argc > 1) {
                        args.append(p2);
                    }
                    c.callv(args);
                    called++;
                    /*
                    if (arg_required != argc) {
                        UtilityFunctions::push_warning(
                                "Method ", c.get_object(), "::", c.get_method(), " should handle ", arg_required,
                                " arguments, but it has ", argc);
                    }*/
                } else {
                    UtilityFunctions::push_error("Callable ", c, " is invalid");
                }
            }
            if (called == 0) {
                UtilityFunctions::push_warning("Command ", command, " was not called due to not matched trains");
            }
        } else {
            log(train_id, TrainLogLevel::TRAINLOGLEVEL_WARNING, "Possibly unknown command");
            WARN_PRINT("Possibly unknown command: " + command);
        }

        train->emit_command_received_signal(command, p1, p2);
    }

    void TrainSystem::broadcast_command(const String &command, const Variant &p1, const Variant &p2) {
        for (auto &[train_id, train]: trains) {
            send_command(train_id, command, p1, p2);
        }
    }
} // namespace godot
