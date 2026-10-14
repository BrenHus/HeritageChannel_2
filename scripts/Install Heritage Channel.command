#!/bin/bash
set -e

script_dir="$(cd "$(dirname "$0")" && pwd)"
source_plugin="$script_dir/Heritage Channel.vst3"
target_dir="$HOME/Library/Audio/Plug-Ins/VST3"

if [ ! -d "$source_plugin" ]; then
    echo "Heritage Channel.vst3 was not found beside this installer."
    read -r -p "Press Return to close."
    exit 1
fi

mkdir -p "$target_dir"
rm -rf "$target_dir/Heritage Channel.vst3"
cp -R "$source_plugin" "$target_dir/"
xattr -cr "$target_dir/Heritage Channel.vst3"
codesign --verify --deep --strict "$target_dir/Heritage Channel.vst3"

echo
echo "Heritage Channel was installed for this user."
echo "Location: $target_dir/Heritage Channel.vst3"
echo "Close and reopen Cubase, then rescan VST3 plug-ins."
echo
read -r -p "Press Return to close."
