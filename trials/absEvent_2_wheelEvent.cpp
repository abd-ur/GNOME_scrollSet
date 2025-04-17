// generalised handler for converting EV_ABS events into REL_WHEEL or RELHWHELL events for virtual device
// ------------------------------------------------------------------------------------------------------

if (ev.type == EV_REL) {
    if (ev.code == REL_WHEEL || ev.code == REL_HWHEEL) {
        int modified_value = static_cast<int>(ev.value * SCROLL_SCALE);
        std::cout << "Modified REL scroll value: " << modified_value << std::endl;
        emit_event(uinput_fd, ev.type, ev.code, modified_value);
        emit_event(uinput_fd, EV_SYN, SYN_REPORT, 0);
    }
} else if (ev.type == EV_ABS) {
    // handle touchpad ABS input 
    std::cout << "ABS event: code=" << ev.code << ", value=" << ev.value << std::endl;

// state-tracker is required for custom scroll detection
