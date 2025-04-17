#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <linux/uinput.h>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <map>

// Parameters
#define SENSITIVITY 0.1
#define MIN_SCROLL_DELTA 10
#define MAX_SCROLL_STEP 5

int setup_uinput_device(int& uinput_fd) {
    uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uinput_fd < 0) {
        perror("Failed to open /dev/uinput");
        return -1;
    }
    ioctl(uinput_fd, UI_SET_EVBIT, EV_REL);
    ioctl(uinput_fd, UI_SET_RELBIT, REL_WHEEL);
    ioctl(uinput_fd, UI_SET_EVBIT, EV_SYN);

    struct uinput_user_dev uidev = {};
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "virtual-scroll");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0x1234;
    uidev.id.product = 0xfedc;
    uidev.id.version = 1;

    write(uinput_fd, &uidev, sizeof(uidev));
    ioctl(uinput_fd, UI_DEV_CREATE);
    sleep(1);
    return 0;
}
void emit_event(int uinput_fd, unsigned int type, unsigned int code, int value) {
    struct input_event ev {};
    gettimeofday(&ev.time, nullptr);
    ev.type = type;
    ev.code = code;
    ev.value = value;
    write(uinput_fd, &ev, sizeof(ev));
}

int main() {
    const char* dev_path = "/dev/input/event0";
    int fd = open(dev_path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Failed to open input device");
        return 1;
    }
    struct libevdev* dev = nullptr;
    if (libevdev_new_from_fd(fd, &dev) < 0) {
        std::cerr << "Failed to init libevdev" << std::endl;
        return 1;
    }
    std::cout << "Monitoring device: " << libevdev_get_name(dev) << std::endl;
    int uinput_fd;
    if (setup_uinput_device(uinput_fd) < 0) {
        return 1;
    }
    int current_slot = 0;
    std::map<int, int> slot_y;
    std::map<int, bool> slot_active;
    bool scrolling_enabled = false;
    int prev_avg_y = -1;

    while (true) {
        struct input_event ev;
        int rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == 0) {
            if (ev.type == EV_ABS) {
                switch (ev.code) {
                    case ABS_MT_SLOT:
                        current_slot = ev.value;
                        break;

                    case ABS_MT_TRACKING_ID:
                        if (ev.value == -1) {
                            slot_active[current_slot] = false;
                            slot_y.erase(current_slot);
                        } else {
                            slot_active[current_slot] = true;
                        }
                        break;

                    case ABS_MT_POSITION_Y:
                        if (slot_active[current_slot]) {
                            slot_y[current_slot] = ev.value;
                        }

                        // finger count
                        int active_fingers = 0;
                        for (auto& [_, is_active] : slot_active)
                            if (is_active) active_fingers++;

                        if (active_fingers >= 2) {
                            // average yPos
                            int sum_y = 0, count = 0;
                            for (auto& [slot, y] : slot_y) {
                                if (slot_active[slot]) {
                                    sum_y += y;
                                    count++;
                                }
                            }

                            if (count >= 2) {
                                int avg_y = sum_y / count;

                                if (prev_avg_y != -1) {
                                    int dy = avg_y - prev_avg_y;
                                    if (std::abs(dy) >= MIN_SCROLL_DELTA) {
                                        int scroll_val = std::clamp(static_cast<int>(-dy * SENSITIVITY), -MAX_SCROLL_STEP, MAX_SCROLL_STEP);
                                        if (scroll_val != 0) {
                                            emit_event(uinput_fd, EV_REL, REL_WHEEL, scroll_val);
                                            emit_event(uinput_fd, EV_SYN, SYN_REPORT, 0);
                                            std::cout << "[Scroll] dy: " << dy << " → scroll: " << scroll_val << std::endl;
                                        }
                                    }
                                }
                                prev_avg_y = avg_y;
                            }
                        } else {
                            prev_avg_y = -1; // reset count
                        }
                        break;
                }
            }
        } else {
            usleep(1000);
        }
    }
    libevdev_free(dev);
    close(fd);
    return 0;
}
