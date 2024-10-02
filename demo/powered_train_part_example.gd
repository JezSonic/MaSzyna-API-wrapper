extends PoweredTrainPart
var _t = 0.0

#func _enter_tree():
#    print("GTC Node ", get_train_controller_node())
#    #bind_command("lock_power", self._on_power_lock)
#    pass

func _process_powered(delta):
    _t += delta
    if _t > 2.0:
        _t = 0.0
        print("Powered train part example: POWERED")

func _process_unpowered(delta):
    _t += delta
    if _t > 2.0:
        _t = 0.0
        print("Powered train part example: UNPOWERED")

func _on_power_lock():
    print("On power lock!")
