#ifndef TRAINSYSTEM_H
#define TRAINSYSTEM_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <map>

namespace godot {

    class TrainController;

    class TrainSystem : public RefCounted {
            GDCLASS(TrainSystem, RefCounted);

        private:
            std::map<String, TrainController *> trains;
            Dictionary commands;

        public:
            TrainSystem();
            ~TrainSystem() override = default;

            void register_train(String id, TrainController *train);
            void bind_command(String train_id, const String &command, const Callable &callback);
            void unbind_command(String train_id, const String &command, const Callable &callback);
            Array get_supported_commands();
            Array get_registered_trains();
            void unregister_train(String train_id);
            void send_command_to_train(
                    String train_id, const String &command, const Variant &p1 = Variant(),
                    const Variant &p2 = Variant());
            void broadcast_command(const String &command, const Variant &p1 = Variant(), const Variant &p2 = Variant());

            bool is_command_supported(const String &command);

        protected:
            static void _bind_methods();
    };
} // namespace godot
#endif // TRAINSYSTEM_H
