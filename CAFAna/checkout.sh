#!/bin/bash

### Check there's no MRB software causing trouble ###

env | grep MRB
if [ $? == 0 ]
then
    echo You already have some MRB software set up? Please start again in a fresh session
    exit 1
fi

### Initial setup and build of novasoft subset ###

rm -f nova_srt_bootstrap
wget https://cdcvs.fnal.gov/redmine/projects/novaart/repository/raw/trunk/SRT_NOVA/scripts/nova_srt_bootstrap || exit 1

chmod +x nova_srt_bootstrap || exit 1

./nova_srt_bootstrap . || exit 1

source srt/srt.sh || exit 1

cat <<EOF > setup/packages-development
OscLib : HEAD
SoftRelTools : HEAD
SRT_NOVA : HEAD
Utilities : HEAD
setup : HEAD
EOF

rm -f update-release || exit 1
wget https://cdcvs.fnal.gov/redmine/projects/novaart/repository/raw/trunk/SRT_NOVA/scripts/update-release || exit 1

chmod +x update-release || exit 1

./update-release -rel development || exit 1

export CVMFS_DISTRO_BASE=/cvmfs/nova.opensciencegrid.org/ || exit 1
source setup/setup_nova.sh -b maxopt -6 $SRT_DIST -e $CVMFS_DISTRO_BASE/externals/ || exit 1

### Editing of a few problematic packages ###

cd releases/development/

rm Utilities/* # lots of art-depending stuff
cd Utilities
svn up rootlogon.C
cd ..

# Don't build the art stuff, just the functions subdir
cat <<EOF > Utilities/GNUmakefile
SUBDIRS := func
include SoftRelTools/standard.mk
EOF

cat <<EOF > OscLib/GNUmakefile
SUBDIRS := func
include SoftRelTools/standard.mk
EOF


### Link our own stuff in ###

ln -s ../../StandardRecord .
ln -s ../../CAFAna .
cd include/
ln -s ../StandardRecord .
ln -s ../CAFAna/
cd ..


### Do the initial build ###

# Force SL6 here
export SRT_ARCH=Linux2.6

time SRT_NOVA/scripts/novasoft_build -rel development

# Put the user in the directory they probably want
cd releases/development/CAFAna/

echo
echo 'In future, type "source setup_cafana.sh" to use this release'
