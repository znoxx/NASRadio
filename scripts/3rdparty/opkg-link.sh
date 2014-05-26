#!/bin/sh
COMMAND=$1
PACKAGE=$2
setdest () {
        for i in `grep dest /etc/opkg.conf | cut -d ' ' -f 3`; do
                if [ -f $i/usr/lib/opkg/info/$PACKAGE.list ]; then
                        DEST=$i
                fi
        done
        if [ "x$DEST" = "x" ]; then
                echo "Can not locate $PACKAGE."
                echo "Check /etc/opkg.conf for correct dest listings";
                echo "Check name of requested package: $PACKAGE"
                exit 1
        fi
}
addlinks () {
        setdest;
        cat $DEST/usr/lib/opkg/info/$PACKAGE.list | while read LINE; do
                SRC=$LINE
                DST=`echo $SRC | sed "s|$DEST||"`
                DSTNAME=`basename $DST`
                DSTDIR=`echo $DST | sed "s|$DSTNAME\$||"`
                test -f "$SRC"
                if [ $? = 0 ]; then
                        test -e "$DST"
                        if [ $? = 1 ]; then
                                mkdir -p $DSTDIR
                                ln -sf $SRC $DST
                        else
                                echo "Not linking $SRC to $DST"
                                echo "$DST Already exists"
                        fi
                else
                        test -d "$SRC"
                        if [ $? = 0 ]; then
                                test -e $DST
                                if [ $? = 1 ]; then
                                        mkdir -p $DST
                                else
                                        echo "directory already exists"
                                fi
                        else
                                echo "Source directory $SRC does not exist"
                        fi
                fi
        done
}
removelinks () {
        setdest;
        cat $DEST/usr/lib/opkg/info/$PACKAGE.list | while read LINE; do
                SRC=$LINE
                DST=`echo $LINE | sed "s|$DEST||"`
                DSTNAME=`basename $DST`
                DSTDIR=`echo $DST | sed "s|$DSTNAME\$||"`
                test -f $DST
                if [ $? = 0 ]; then
                        rm -f $DST
                        test -d $DSTDIR && rmdir $DSTDIR 2>/dev/null
                else
                        test -d $DST
                        if [ $? = 0 ]; then
                                rmdir $DST
                        else
                                echo "$DST does not exist"
                        fi
                fi
        done
}
mountdest () {
        test -d $PACKAGE
        if [ $? = 1 ]; then
                echo "Mount point does not exist"
                exit 1
        fi
        for i in $PACKAGE/usr/lib/opkg/info/*.list; do
                $0 add `basename $i .list`
        done
}
umountdest () {
        test -d $PACKAGE
        if [ $? = 1 ]; then
                echo "Mount point does not exist"
                exit 1
        fi
        for i in $PACKAGE/usr/lib/opkg/info/*.list; do
                $0 remove `basename $i .list`
        done
}
case "$COMMAND" in
  add)
        addlinks
  ;;
  remove)
        removelinks
  ;;
  mount)
        mountdest
  ;;
  umount)
        umountdest
  ;;
  *)
        echo "Usage: $0 <cmd> <target>"
        echo "       Commands: add, remove, mount, umount"
        echo "       Targets: <package>, <mount point>"
        echo "Example:  $0 add kismet-server"
        echo "Example:  $0 remove kismet-server"
        echo "Example:  $0 mount /mnt/usb"
        echo "Example:  $0 umount /mnt/usb"
        exit 1
        ;;
esac
exit 0
