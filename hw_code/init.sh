curl -s "https://misc0110.github.io/ppa/KEY.gpg" | sudo tee /etc/apt/trusted.gpg.d/pteditor.asc
sudo curl -s -o /etc/apt/sources.list.d/misc0110.list "https://misc0110.github.io/ppa/file.list"
sudo apt update
sudo apt install pteditor-dkms

cd ../ PTEditor
sudo apt install linux-headers-$(uname -r)
make
sudo insmod module/pteditor.ko
cd ../hw_code