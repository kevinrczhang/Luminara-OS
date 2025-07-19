#!/bin/bash
# Simple Donkey OS Setup Script
# Prerequisites: VirtualBox installed on Windows, WSL setup

set -e

echo "Setting up Donkey OS..."

# Install build tools if missing
sudo apt update
sudo apt install -y build-essential gcc-multilib g++-multilib grub-pc-bin grub-common xorriso

# VirtualBox paths
VBOX_PATH="/mnt/c/Program Files/Oracle/VirtualBox"
VM_NAME="Donkey OS"

# Remove existing VM if it exists
"$VBOX_PATH/VBoxManage.exe" controlvm "$VM_NAME" poweroff 2>/dev/null || true
"$VBOX_PATH/VBoxManage.exe" unregistervm "$VM_NAME" --delete 2>/dev/null || true

# Create new VM
"$VBOX_PATH/VBoxManage.exe" createvm --name "$VM_NAME" --ostype "Other" --register
"$VBOX_PATH/VBoxManage.exe" modifyvm "$VM_NAME" --memory 512 --vram 16
"$VBOX_PATH/VBoxManage.exe" modifyvm "$VM_NAME" --mouse ps2 --keyboard ps2
"$VBOX_PATH/VBoxManage.exe" modifyvm "$VM_NAME" --boot1 dvd
"$VBOX_PATH/VBoxManage.exe" storagectl "$VM_NAME" --name "IDE Controller" --add ide

make clean
make

echo "Creating ISO..."
mkdir -p iso_dir/boot/grub
cp build/kernel.bin iso_dir/boot/kernel.bin
cp grub.cfg iso_dir/boot/grub/
grub-mkrescue -o build/os.iso iso_dir
rm -rf iso_dir
echo "ISO created: build/os.iso"

# Create run script
cat > run_vm.sh << 'EOF'
#!/bin/bash
VBOX_PATH="/mnt/c/Program Files/Oracle/VirtualBox"
VM_NAME="Donkey OS"
WIN_ISO_PATH="$(wslpath -w "$(pwd)/build/os.iso")"

"$VBOX_PATH/VBoxManage.exe" controlvm "$VM_NAME" poweroff 2>/dev/null || true
sleep 2
"$VBOX_PATH/VBoxManage.exe" storageattach "$VM_NAME" --storagectl "IDE Controller" --port 0 --device 0 --type dvddrive --medium "$WIN_ISO_PATH"
"$VBOX_PATH/VirtualBoxVM.exe" --startvm "$VM_NAME" &
EOF

chmod +x run_vm.sh

# Start VM
./run_vm.sh

echo "Done! VM started. Click in VirtualBox window and move mouse to test."
