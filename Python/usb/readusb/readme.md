Read USB packet (for XBOX 360 analysis)
----

## Preparing
Do install as follows, for `import usb.core`.
```
pip3 install usb
pip3 install pyusb
apt install python3-usb
```



## How to use
sudo python3 readpy.py

and do fiddle the XBOX controller to hard.

```text

epaddr= 129
r= array('B', [1, 3, 14]) 3
r= array('B', [2, 3, 0]) 3
r= array('B', [3, 3, 3]) 3
r= array('B', [8, 3, 0]) 3
r= array('B', [0, 20, 0, 0, 0, 0, 183, 1, 192, 7, 14, 5, 61, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 42, 11, 1, 9, 14, 5, 61, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 8, 15, 223, 14, 14, 5, 61, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 37, 2, 247, 16, 122, 5, 61, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 79, 202, 100, 229, 14, 5, 172, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 223, 198, 173, 129, 14, 5, 172, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 147, 2, 0, 128, 14, 5, 61, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 162, 29, 74, 15, 14, 5, 172, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 219, 0, 223, 14, 14, 5, 61, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 211, 241, 105, 172, 14, 5, 172, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 248, 51, 0, 128, 14, 5, 61, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 154, 14, 50, 13, 14, 5, 172, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 80, 13, 240, 11, 14, 5, 172, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 190, 13, 174, 10, 14, 5, 61, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 190, 13, 160, 0, 14, 5, 61, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 190, 13, 236, 249, 14, 5, 61, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 193, 19, 219, 252, 14, 5, 172, 6, 0, 0, 0, 0, 0, 0]) 20
r= array('B', [0, 20, 0, 0, 0, 0, 219, 0, 50, 13, 14, 5, 172, 6, 0, 0, 0, 0, 0, 0]) 20
Traceback (most recent call last):
  File "readusb.py", line 28, in <module>
    r=dev.read(eaddr, 1024)
  File "/usr/lib/python3/dist-packages/usb/core.py", line 983, in read
    ret = fn(
  File "/usr/lib/python3/dist-packages/usb/backend/libusb1.py", line 846, in intr_read
    return self.__read(self.lib.libusb_interrupt_transfer,
  File "/usr/lib/python3/dist-packages/usb/backend/libusb1.py", line 936, in __read
    _check(retval)
  File "/usr/lib/python3/dist-packages/usb/backend/libusb1.py", line 595, in _check
    raise USBError(_strerror(ret), ret, _libusb_errno[ret])
usb.core.USBError: [Errno 110] Operation timed out
```

The last time out cause didn't issue interrupt from XBOX 360 due to didn't input from user operation.
The packet was canonically and sucseeded captureing.

## LICENSE

NYSL : http://www.kmonos.net/nysl/index.en.html

