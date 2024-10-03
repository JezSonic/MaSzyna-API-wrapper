#include "../core/TrainController.hpp"
#include "TrainSystem.hpp"

namespace godot {
    void TrainSystem::_bind_methods() {
        ClassDB::bind_method(D_METHOD("register_train", "train"), &TrainSystem::register_train);
        ClassDB::bind_method(D_METHOD("unregister_train", "train"), &TrainSystem::unregister_train);
        ClassDB::bind_method(
                D_METHOD("send_command_to_train", "train_id", "command", "p1", "p2"),
                &TrainSystem::send_command_to_train);
        ClassDB::bind_method(
                D_METHOD("broadcast_command", "command", "p1", "p2"), &TrainSystem::broadcast_command,
                DEFVAL(Variant()), DEFVAL(Variant()));
        ClassDB::bind_method(D_METHOD("is_command_supported", "command"), &TrainSystem::is_command_supported);
        ClassDB::bind_method(D_METHOD("get_supported_commands"), &TrainSystem::get_supported_commands);
        ClassDB::bind_method(D_METHOD("bind_command", "train", "command", "callable"), &TrainSystem::bind_command);
        ClassDB::bind_method(D_METHOD("unbind_command", "train", "command", "callable"), &TrainSystem::unbind_command);
    }

    TrainSystem::TrainSystem() {}

    void TrainSystem::register_train(TrainController *train) {
        unsigned long train_id = train->get_instance_id(); // Get the unique unsigned long
        trains[train_id] = train;
    }

    void TrainSystem::bind_command(TrainController *train, const String &command, const Callable &callback) {

        if (trains.find(train->get_instance_id()) == trains.end()) {
            UtilityFunctions::push_error("Train is not registered in TrainSystem: ", train);
            return;
        }

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

    void TrainSystem::unbind_command(TrainController *train, const String &command, const Callable &callback) {
        if (trains.find(train->get_instance_id()) == trains.end()) {
            UtilityFunctions::push_error("Train is not registered in TrainSystem: ", train);
            return;
        }

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

    void TrainSystem::unregister_train(TrainController *train) {
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

        trains.erase(instance_id);
    }

    bool TrainSystem::is_command_supported(const String &command) {
        return commands.has(command);
    }

    Array TrainSystem::get_supported_commands() {
        return commands.keys();
    }

    void TrainSystem::send_command_to_train(
            TrainController *train, const String &command, const Variant &p1, const Variant &p2) {
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
                    args.append(p1);
                    args.append(p2);
                    c.callv(args);
                    called++;
                } else {
                    UtilityFunctions::push_error("Callable ", c, " is invalid");
                }
            }
            if (called == 0) {
                UtilityFunctions::push_warning("Command ", command, " was not called due to not matched trains");
            }
        } else {
            WARN_PRINT("Possibly unknown command: " + command);
        }

        train->_on_command_received(command, p1, p2);
        train->emit_command_received_signal(command, p1, p2);
    }

    void TrainSystem::broadcast_command(const String &command, const Variant &p1, const Variant &p2) {
        for (auto &[train_id, train]: trains) {
            send_command_to_train(train, command, p1, p2);
        }
    }
} // namespace godot
