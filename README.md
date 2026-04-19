# QT-Practice

Qt + OpenCV practice project for a simple desktop vision app.

## Docker Development Environment

If you want to build inside the provided Docker image:

```sh
docker build -t rpi-dev .
docker run -it --rm \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v $(pwd):/app \
  rpi-dev bash
```

Then build the app inside the container:

```sh
mkdir build
cd build
cmake ..
make
./my_vision_app
```
