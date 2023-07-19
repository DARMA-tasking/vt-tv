FROM pierrpebay/vt-tv:latest AS build

# build

FROM build AS test

RUN python3 -c 'print("Hello World! (From build and test)")'

# test
