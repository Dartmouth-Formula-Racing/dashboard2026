# dashboard2026

talia data visualisation based on Dashboard_Broadcast_Task() in [misc.c](https://github.com/Dartmouth-Formula-Racing/CVC-2025/blob/cvc-2026/src/misc.c)

Nucleo-F446RE with SSD1963 800x480 LCD + MCP2515 CAN shield

## Build targets / mode switch

Run native test mode:

```bash
pio run -e native
pio run -e native -t run
```

Run hardware mode:

```bash
pio run -e nucleo_f446re
pio run -e nucleo_f446re -t upload
```