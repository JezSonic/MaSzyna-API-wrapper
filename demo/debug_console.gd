extends Node

@onready var console = $Console
@export var visible:bool = false:
    set(x):
        if console:
            console.control.visible = x
        visible = x


func _ready() -> void:
    console.control.visible = visible
    console.add_command("broadcast", self.console_broadcast, ["command", "p1", "p2"], 1)
    console.add_command("send", self.console_send, ["train", "command", "p1", "p2"], 2)
    console.add_command("trains", self.console_list_trains)
    console.add_command("traincommands", self.console_list_train_commands)
    console.add_command("state", self.console_state, ["train", "parameter"], 1)

    TrainSystem.train_log_updated.connect(self.console_print_train_log)

func console_broadcast(command, p1=null, p2=null):
    #Console.print_line("Broadcasting command: %s(%s, %s)" % [command, p1, p2], true)
    TrainSystem.broadcast_command(command, p1, p2)

func console_send(train, command, p1=null, p2=null):
    TrainSystem.send_command_to_train(train, command, p1, p2)

func console_list_trains():
    console.print_line("%s" % "\n".join(TrainSystem.get_registered_trains()))

func console_list_train_commands():
    console.print_line("%s" % "\n".join(TrainSystem.get_supported_commands()))

func console_print_train_log(train_id, loglevel, line):
    if loglevel >= TrainSystem.TrainLogLevel.TRAINLOGLEVEL_ERROR:
        console.print_line("[color=red]%s: %s[/color]" % [train_id, line])
    elif loglevel == TrainSystem.TrainLogLevel.TRAINLOGLEVEL_WARNING:
        console.print_line("[color=orange]%s: %s[/color]" % [train_id, line])
    else:
        console.print_line("%s: %s" % [train_id, line])

func console_state(train, key=null):
    var out = TrainSystem.get_train_state(train)
    if key:
        out = out.get(key)
    console.print_line("%s" % [out])
