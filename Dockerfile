FROM envoyproxy/envoy-build-ubuntu:7304f974de2724617b7492ccb4c9c58cd420353a
COPY . /src
WORKDIR /src
CMD ["/src/build.sh", ""]
