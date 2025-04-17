## Working prototype to configure touchpad scrolling sensitivity for Asahi Fedora GNOME running on Mac m1

---

### Features
- Detects two-finger gesture using raw `ABS_MT_POSITION_Y`
- Emits synthetic vertical scroll (`REL_WHEEL`) events
- Adjusts sensitivity with a configurable factor along with
- MIX/MAX Delta adjustment for optimized scroll activation

---

### Requirements

- **Asahi Linux Fedora GNOME**
- C++ compiler (`g++`)
- [`libevdev`](https://www.freedesktop.org/wiki/Software/libevdev/)
- Write access to:
  - `/dev/input/eventX`
  - `/dev/uinput`

---

### Build Instructions

```bash
sudo dnf install libevdev-devel
g++ scroll.cpp -o scroll -levdev
```

---

### Run

```bash
sudo ./scroll
```

> Adjust `dev_path` in `scroll.cpp` if your trackpad is not `event0`.

---

### Configuration

In `scroll.cpp`, tune these constants to adjust scroll behavior:

```cpp
#define SENSITIVITY 0.1
#define MIN_SCROLL_DELTA 10
#define MAX_SCROLL_STEP 5
```

---

### Known Issues

- Scroll activates even without significant finger movement
- No gesture state tracking yet (finger down/up)
- Inaccurate sensitivity processing

---

### TODO

- Improve gesture detection
- Ignore accidental scrolls
- Configurable via CLI or config file
- Add horizontal scroll support

---

### Debugging

To monitor raw touchpad events:

```bash
sudo evtest /dev/input/eventX
```

---

### Contributing

Pull requests and issues are welcome!  

---

### License

MIT License — use it freely, contribute if you'd like 💙
