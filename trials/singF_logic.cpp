int y_prev = -1;
int y_curr = -1;
bool scroll_detected = false;

while (true) {
    struct input_event ev;
    int rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

    if (rc == 0) {
        if (ev.type == EV_ABS && ev.code == ABS_MT_POSITION_Y) {
            y_prev = y_curr;
            y_curr = ev.value;

            if (y_prev != -1) {
                int dy = y_curr - y_prev;
                if (abs(dy) > threshold) {
                    int scaled_dy = dy * SENSITIVITY;
                    emit_event(uinput_fd, EV_REL, REL_WHEEL, -scaled_dy);
                    emit_event(uinput_fd, EV_SYN, SYN_REPORT, 0);
                    std::cout << "Scroll detected! dy: " << dy << " | scaled: " << scaled_dy << std::endl;
                }
            }
        }
    }
}
