extends Node


@export var visible:bool = false:
    set(x):
        if Console:
            Console.set_visible(x)
        visible = x


func _ready() -> void:
    Console.control.visible = visible
    Console.add_command("broadcast", self.console_broadcast, ["command", "p1", "p2"], 1, "Broadcast message to all trains")
    Console.add_command("send", self.console_send, ["train", "command", "p1", "p2"], 2, "Send message to a train")
    Console.add_command("trains", self.console_list_trains, 0, 0, "List trains")
    Console.add_command("commands", self.console_list_train_commands, 0, 0, "List available train commands")
    Console.add_command("get", self.console_get_train_state, ["train", "parameter"], 1, "Get train state / parameter")

    TrainSystem.train_log_updated.connect(self.console_print_train_log)

func console_broadcast(command, p1=null, p2=null):
    #Console.print_line("Broadcasting command: %s(%s, %s)" % [command, p1, p2], true)
    TrainSystem.broadcast_command(command, p1, p2)

func console_send(train, command, p1=null, p2=null):
    TrainSystem.send_command(train, command, p1, p2)

func console_list_trains():
    Console.print_line("%s" % "\n".join(TrainSystem.get_registered_trains()))

func console_list_train_commands():
    var commands = TrainSystem.get_supported_commands()
    commands.sort()
    Console.print_line("%s" % "\n".join(commands))

func console_print_train_log(train_id, loglevel, line):
    if loglevel >= TrainSystem.TrainLogLevel.TRAINLOGLEVEL_ERROR:
        Console.print_line("[color=red]%s: %s[/color]" % [train_id, line])
    elif loglevel == TrainSystem.TrainLogLevel.TRAINLOGLEVEL_WARNING:
        Console.print_line("[color=orange]%s: %s[/color]" % [train_id, line])
    else:
        Console.print_line("%s: %s" % [train_id, line])

func console_get_train_state(train, key=null):
    var out = TrainSystem.get_train_state(train)
    if key:
        out = out.get(key)
    Console.print_line("%s" % [out])
