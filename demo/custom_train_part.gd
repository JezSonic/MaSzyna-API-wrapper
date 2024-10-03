extends GenericTrainPart
class_name CustomTrainPart

var _total_time = 0.0
var state = {
    "custom_train_part_calls": 0,
}

func _process_train_part(delta):
    _total_time += delta
    if _total_time > 2:
        _total_time = 0
        log_debug("Here is a custom train part")
        state["custom_train_part_calls"] += 1

func _get_train_part_state():
    return state
