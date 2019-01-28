i=1
while true;do
    if [ $i -eq 11 ] ;then 
        break 
    fi
    ./lab1 >> junk
    i=`expr $i + 1`    
   
done

if search="$(diff smoothed_conv.ppm smoothed_sep.ppm| grep 'diff')";then
    echo "$search"
else
    echo "conv&sep identical"
fi

if search="$(diff smoothed_sw.ppm smoothed_sep.ppm| grep 'diff')";then
    echo "$search"
else
    echo "sw&sep identical"
fi

if search="$(diff smoothed_conv.ppm smoothed_sw.ppm| grep 'diff')";then
    echo "$search"
else
    echo "conv&sw identical"
fi

python rdata.py
