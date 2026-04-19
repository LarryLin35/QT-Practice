```sh
wsl
docker version
docker context ls
docker ps

docker build -t rpi-dev .

docker run -it --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v $(pwd):/app \
  rpi-dev bash

mkdir build && cd build
cmake ..
make # 重新編譯可用 make && ./my_vision_app
./my_vision_app
```