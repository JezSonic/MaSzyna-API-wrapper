extends Node

@onready var console = $Console

func _ready() -> void:
    console.add_command("broadcast", self.console_broadcast, ["command", "p1", "p2"], 1)
    console.add_command("send", self.console_send, ["train", "command", "p1", "p2"], 2)
    console.add_command("trains", self.console_list_trains)
    console.add_command("traincommands", self.console_list_train_commands)

func console_broadcast(command, p1=null, p2=null):
    #Console.print_line("Broadcasting command: %s(%s, %s)" % [command, p1, p2], true)
    TrainSystem.broadcast_command(command, p1, p2)

func console_send(train, command, p1=null, p2=null):
    TrainSystem.send_command_to_train(train, command, p1, p2)

func console_list_trains():
    console.print_line("%s" % "\n".join(TrainSystem.get_registered_trains()))

func console_list_train_commands():
    console.print_line("%s" % "\n".join(TrainSystem.get_supported_commands()))
