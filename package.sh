# Packages the sources and the compiled binaries

NAME="sulzbach-ferrazza.zip"
HELPER="packaging-helper"
# Renaming the executable is necessary because the executable `game` conflicts
# with the path `game` when both are moved to the same directory.
EXECUTABLE="walls-of-doom"

# Delete the old file if it exists
rm -f $NAME

# Zip the current tree, except for .git, blender, and build
zip -q -r $NAME . -x .\* blender/\* build/\*

# Create the helper directory
mkdir $HELPER
cd $HELPER
# Build everything in the helper directory
cmake ..
make
cd ..
# Add the executable to the archive
mv $HELPER/game/game $HELPER/$EXECUTABLE
zip -q -j $NAME $HELPER/$EXECUTABLE
# Clean the helper directory up
rm -rf $HELPER

# Create a PDF from the README using Pandoc
bash readme-to-pdf.sh
zip -q $NAME README.pdf
rm README.pdf
