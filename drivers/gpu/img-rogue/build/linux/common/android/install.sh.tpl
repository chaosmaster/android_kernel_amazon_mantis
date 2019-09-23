#!/bin/bash
############################################################################ ###
#@Copyright     Copyright (c) Imagination Technologies Ltd. All Rights Reserved
#@License       Dual MIT/GPLv2
# 
# The contents of this file are subject to the MIT license as set out below.
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# Alternatively, the contents of this file may be used under the terms of
# the GNU General Public License Version 2 ("GPL") in which case the provisions
# of GPL are applicable instead of those above.
# 
# If you wish to allow use of your version of this file only under the terms of
# GPL, and not to allow others to use your version of this file under the terms
# of the MIT license, indicate your decision by deleting the provisions above
# and replace them with the notice and other provisions required by GPL as set
# out in the file called "GPL-COPYING" included in this distribution. If you do
# not delete the provisions above, a recipient may use your version of this file
# under the terms of either the MIT license or GPL.
# 
# This License is also included in this distribution in the file called
# "MIT-COPYING".
# 
# EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
# PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
# BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#### ###########################################################################
# Help on how to invoke
#
function usage {
    echo "usage: $0 [options...]"
    echo ""
    echo "Options: -v            Verbose mode."
    echo "         -n            Dry-run mode."
    echo "         -u            Uninstall-only mode."
    echo "         --root <path> Use <path> as the root of the install file system."
    echo "                       (Overrides the DISCIMAGE environment variable.)"
    echo "         -p <target>   Pack mode: Don't install anything.  Just copy files"
    echo "                       required for installation to <target>." 
    echo "                       (Sets/overrides the PACKAGEDIR environment variable.)"
    exit 1
}

WD=`pwd`
SCRIPT_ROOT=`dirname $0`
cd $SCRIPT_ROOT

PVRVERSION=[PVRVERSION]
PVRBUILD=[PVRBUILD]
PRIMARY_ARCH="[PRIMARY_ARCH]"
ARCHITECTURES="[ARCHITECTURES]"

# These destination directories are the same for 32- or 64-bit binaries.

APP_DESTDIR=[APP_DESTDIR]
BIN_DESTDIR=[BIN_DESTDIR]
FW_DESTDIR=[FW_DESTDIR]
DATA_DESTDIR=[BIN_DESTDIR]
TEST_DESTDIR=[TEST_DESTDIR]

# Exit with an error messages.
# $1=blurb
#
function bail {
    if [ ! -z "$1" ]; then
        echo "$1" >&2
    fi

    echo "" >&2
    echo "Installation failed" >&2
    exit 1
}

# Copy the files that we are going to install into $PACKAGEDIR
function copy_files_locally() {
    # Create versions of the installation functions that just copy files to a useful place.
    function check_module_directory() { true; }
    function uninstall() { true; }
    function link_library() { true; }
    function symlink_library_if_not_present() { true; }

    # basic installation function
    # $1=fromfile, $4=chmod-flags
    # plus other stuff that we aren't interested in.
    function install_file() {
        if [ -f "$1" ]; then
            $DOIT cp $1 $PACKAGEDIR/$THIS_ARCH
            $DOIT chmod $4 $PACKAGEDIR/$THIS_ARCH/$1
        fi
    }
    
    # Tree-based installation function
    # $1 = fromdir
    # plus other stuff that we aren't interested in.
    function install_tree() {
        if [ -d "$1" ]; then
            cp -Rf $1 $PACKAGEDIR/$THIS_ARCH
        fi
    }

    echo "Copying files to $PACKAGEDIR."
    
    if [ -d $PACKAGEDIR ]; then
        rm -Rf $PACKAGEDIR
    fi
    mkdir -p $PACKAGEDIR

    for THIS_ARCH in $ARCHITECTURES; do
        if [ ! -d $THIS_ARCH ]; then
            continue
        fi

        mkdir -p $PACKAGEDIR/$THIS_ARCH
        pushd $THIS_ARCH > /dev/null
        if [ -f install_um.sh ]; then
            source install_um.sh
            install_file install_um.sh x x 0644
        fi
        install_file rgxfw_debug.zip x x 0644
        popd > /dev/null
    done
    
    THIS_ARCH=$PRIMARY_ARCH
    pushd $THIS_ARCH > /dev/null
    if [ -f install_km.sh ]; then
        source install_km.sh
        install_file install_km.sh x x 0644
    fi
    popd > /dev/null

    unset THIS_ARCH
    install_file install.sh x x 0755
}

# Install the files on the remote machine using SSH
# We do this by:
#  - Copying the required files to a place on the local disk
#  - rsync these files to the remote machine
#  - run the install via SSH on the remote machine
function install_via_ssh() {
    # Default to port 22 (SSH) if not otherwise specified
    if [ -z "$INSTALL_TARGET_PORT" ]; then
        INSTALL_TARGET_PORT=22
    fi

    # Execute something on the target machine via SSH
    # $1 The command to execute
    function remote_execute() {
        COMMAND=$1
        ssh -p "$INSTALL_TARGET_PORT" -q -o "BatchMode=yes" root@$INSTALL_TARGET "$1"
    }

    if ! remote_execute "test 1"; then
        echo "Can't access $INSTALL_TARGET via ssh."
        echo "Have you installed your public key into root@$INSTALL_TARGET:~/.ssh/authorized_keys?"
        echo "If root has a password on the target system, you can do so by executing:"
        echo "ssh root@$INSTALL_TARGET \"mkdir -p .ssh; cat >> .ssh/authorized_keys\" < ~/.ssh/id_rsa.pub"
        bail
    fi

    # Create a directory to contain all the files we are going to install.
    PACKAGEDIR_PREFIX=`mktemp -d` || bail "Couldn't create local temporary directory"
    PACKAGEDIR=$PACKAGEDIR_PREFIX/Rogue_DDK_Install_Root
    PACKAGEDIR_REMOTE=/tmp/Rogue_DDK_Install_Root
    copy_files_locally

    echo "RSyncing $PACKAGEDIR to $INSTALL_TARGET:$INSTALL_TARGET_PORT."
    $DOIT rsync -crlpt -e "ssh -p \"$INSTALL_TARGET_PORT\"" --delete $PACKAGEDIR/ root@$INSTALL_TARGET:$PACKAGEDIR_REMOTE || bail "Couldn't rsync $PACKAGEDIR to root@$INSTALL_TARGET"
    echo "Running "$INSTALL_PREFIX"nstall remotely."

    REMOTE_COMMAND="bash $PACKAGEDIR_REMOTE/install.sh -r /"

    if [ "$UNINSTALL_ONLY" == "y" ]; then
    REMOTE_COMMAND="$REMOTE_COMMAND -u"
    fi

    remote_execute "$REMOTE_COMMAND"
    rm -Rf $PACKAGEDIR_PREFIX
}

# Copy all the required files into their appropriate places on the local machine.
function install_locally {
    # Define functions required for local installs

    # basic installation function
    # $1=fromfile, $2=destfilename, $3=blurb, $4=chmod-flags, $5=chown-flags
    #
    function install_file {
        if [ -z "$DDK_INSTALL_LOG" ]; then
            bail "INTERNAL ERROR: Invoking install without setting logfile name"
        fi
        DESTFILE=${DISCIMAGE}$2
        DESTDIR=`dirname $DESTFILE`
    
        if [ ! -e $1 ]; then
            [ -n "$VERBOSE" ] && echo "skipping file $1 -> $2"
            return
        fi
        
        # Destination directory - make sure it's there and writable
        #
        if [ -d "${DESTDIR}" ]; then
            if [ ! -w "${DESTDIR}" ]; then
                bail "${DESTDIR} is not writable."
            fi
        else
            $DOIT mkdir -p ${DESTDIR} || bail "Couldn't mkdir -p ${DESTDIR}"
            [ -n "$VERBOSE" ] && echo "Created directory `dirname $2`"
        fi
    
        # Delete the original so that permissions don't persist.
        #
        $DOIT rm -f $DESTFILE
    
        $DOIT cp -f $1 $DESTFILE || bail "Couldn't copy $1 to $DESTFILE"
        $DOIT chmod $4 ${DESTFILE}
    
        echo "$3 `basename $1` -> $2"
        $DOIT echo "file $2" >> $DDK_INSTALL_LOG
    }

    # If we install to an empty $DISCIMAGE, then we need to create some
    # dummy directories, even if they contain no files, otherwise 'adb
    # sync' may fail. (It allows '/vendor' to not exist currently.)
    [ ! -d ${DISCIMAGE}/data ]   && mkdir ${DISCIMAGE}/data
    [ ! -d ${DISCIMAGE}/system ] && mkdir ${DISCIMAGE}/system

    for arch in $ARCHITECTURES; do
        if [ ! -d $arch ]; then
            echo "Missing architecture $arch"
            if [ "$arch" = "$PRIMARY_ARCH" ]; then
                echo "Primary architecture is missing, aborting!"
                exit 1
            else
                continue
            fi
        fi

        BASE_DESTDIR=`dirname ${BIN_DESTDIR}`
        case $arch in
            target*64*)
                SHLIB_DESTDIR=${BASE_DESTDIR}/lib64
                ;;
            *)
                SHLIB_DESTDIR=${BASE_DESTDIR}/lib
                ;;
        esac
        EGL_DESTDIR=${SHLIB_DESTDIR}/egl

        pushd $arch > /dev/null
        # Install UM components
        if [ -f install_um.sh ]; then
            DDK_INSTALL_LOG=$UMLOG
            echo "Installing User components for architecture $arch"
            $DOIT echo "version $PVRVERSION" > $DDK_INSTALL_LOG
            source install_um.sh
            echo 
        fi
        popd > /dev/null
    done

    pushd $PRIMARY_ARCH > /dev/null
    # Install KM components
    if [ -f install_km.sh ]; then
        DDK_INSTALL_LOG=$KMLOG
        echo "Installing Kernel components for architecture $PRIMARY_ARCH"
        $DOIT echo "version $PVRVERSION" > $DDK_INSTALL_LOG
        source install_km.sh
        echo
    fi
    popd > /dev/null

    # Create an OLDLOG so old versions of the driver can uninstall.
    $DOIT echo "version $PVRVERSION" > $OLDLOG
    if [ -f $KMLOG ]; then
        tail -n +2 $KMLOG >> $OLDLOG
    fi
    if [ -f $UMLOG ]; then
        tail -n +2 $UMLOG >> $OLDLOG
    fi
    
    # Make sure new logs are newer than $OLDLOG
    touch -m -d "last sunday" $OLDLOG
}

# Read the appropriate install log and delete anything therein.
function uninstall_locally {
    # Function to uninstall something.
    function do_uninstall {
        LOG=$1

        if [ ! -f $LOG ]; then
            echo "Nothing to un-install."
            return;
        fi
    
        BAD=0
        VERSION=""
        while read type data; do
            case $type in
            version)
                echo "Uninstalling existing version $data"
                VERSION="$data"
                ;;
            link|file) 
                if [ -z "$VERSION" ]; then
                    BAD=1;
                    echo "No version record at head of $LOG"
                elif ! $DOIT rm -f ${DISCIMAGE}${data}; then
                    BAD=1;
                else
                    [ -n "$VERBOSE" ] && echo "Deleted $type $data"
                fi
                ;;
            tree)
                ;;
            esac
        done < $1;

        if [ $BAD = 0 ]; then
            echo "Uninstallation completed."
            $DOIT rm -f $LOG
        else
            echo "Uninstallation failed!!!"
        fi
    }

    if [ -z "$OLDLOG" -o -z "$KMLOG" -o -z "$UMLOG" ]; then
        bail "INTERNAL ERROR: Invoking uninstall without setting logfile name"
    fi

    # Uninstall anything installed using the old-style install scripts.
    LEGACY_LOG=0
    if [ -f $OLDLOG ]; then
        if [ -f $KMLOG -a $KMLOG -nt $OLDLOG ]; then
            # Last install was new scheme.
            rm $OLDLOG
        elif [ -f $UMLOG -a $UMLOG -nt $OLDLOG ]; then
            # Last install was new scheme.
            rm $OLDLOG
        else
            echo "Uninstalling all components from legacy log."
            do_uninstall $OLDLOG
            LEGACY_LOG=1
            echo 
        fi
    fi

    if [ $LEGACY_LOG = 0 ]; then
        # Uninstall KM components if we are doing a KM install.
        if [ -f install_km.sh -a -f $KMLOG ]; then
            echo "Uninstalling Kernel components"
            do_uninstall $KMLOG
            echo 
        fi
        # Uninstall UM components if we are doing a UM install.
        if [ -f install_um.sh -a -f $UMLOG ]; then
            echo "Uninstalling User components"
            do_uninstall $UMLOG
            echo 
        fi
    fi
}

# Work out if there are any special instructions.
#
while [ "$1" ]; do
    case "$1" in
    -v|--verbose)
        VERBOSE=v
        ;;
    -r|--root)
        DISCIMAGE=$2
        shift;
        ;;
    -u|--uninstall)
        UNINSTALL_ONLY=y
        ;;
    -n)
        DOIT=echo
        ;;
    -p|--package)
        PACKAGEDIR=$2
        if [ ${PACKAGEDIR:0:1} != '/' ]; then
            PACKAGEDIR=$WD/$PACKAGEDIR
        fi
        shift;
        ;;
    -h | --help | *)
        usage
        ;;
    esac
    shift
done

if [ ! -z "$PACKAGEDIR" ]; then 
    copy_files_locally $PACKAGEDIR
    echo "Copy complete!"

elif [ ! -z "$DISCIMAGE" ]; then

    if [ ! -d "$DISCIMAGE" ]; then
       bail "$0: $DISCIMAGE does not exist."
    fi

    echo
    if [ $DISCIMAGE == "/" ]; then
        echo "Installing PowerVR '$PVRVERSION ($PVRBUILD)' locally"
    else
        echo "Installing PowerVR '$PVRVERSION ($PVRBUILD)' on $DISCIMAGE"
    fi
    echo
    echo "File system installation root is $DISCIMAGE"
    echo

    OLDLOG=$DISCIMAGE/powervr_ddk_install.log
    KMLOG=$DISCIMAGE/powervr_ddk_install_km.log
    UMLOG=$DISCIMAGE/powervr_ddk_install_um.log

    uninstall_locally

    if [ "$UNINSTALL_ONLY" != "y" ]; then
        install_locally
    fi

else
    bail "DISCIMAGE must be set for installation to be possible."
fi
