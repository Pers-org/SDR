#use this script, if you want run 2 sdr (one tx one rx)

../build/TX_test

#get PLUTO uri
USB_URI_1=$(iio_info -a 2>&1 | grep -oE "usb:[0-9.]+" | head -n 1)
USB_URI_2=$(iio_info -a 2>&1 | grep -oE "usb:[0-9.]+" | tail -n 1)

TX_TIME=5
RX_TIME=1

#run tx
sudo ../build/main $USB_URI_2 1 $TX_TIME &
PID2=$!

sleep 1

#run rx
sudo ../build/main $USB_URI_1 0 $RX_TIME &
PID1=$!

wait $PID1 $PID2
