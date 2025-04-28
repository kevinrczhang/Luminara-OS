Vagrant.configure("2") do |config|
    # Base box and VM resources
    config.vm.box = "generic/ubuntu2204"
    config.vm.provider "virtualbox" do |vb|
      vb.memory = "2048"
      vb.cpus   = 2
      vb.gui    = true
    end
  
    # Sync the project folder
    config.vm.synced_folder ".", "/home/vagrant/osdev"
  
    # Install toolchain, GRUB, QEMU, etc.
    config.vm.provision "shell", inline: <<-SHELL
      sudo apt update
      sudo apt install -y make gcc g++ build-essential bison flex texinfo \\
                          libgmp-dev libmpfr-dev libmpc-dev \\
                          gcc-multilib g++-multilib \\
                          grub-pc-bin xorriso qemu-system-x86 gdb-multiarch
    SHELL
  end
