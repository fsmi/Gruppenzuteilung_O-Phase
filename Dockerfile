FROM debian:12 AS builder
RUN apt-get update && apt-get install -y build-essential git cmake

WORKDIR /build
COPY . /build

RUN \
	mkdir build && \
	cd build && \
	cmake .. && \
	make GroupAssignment

FROM debian:12
RUN apt-get update && apt-get install -y \
	nano \
	mc \
	&& rm -rf /var/lib/apt/lists/*

COPY --from=builder /build/build/GroupAssignment /usr/local/bin/GroupAssignment

# copy source repo into container for easier experimentation
COPY . /source

COPY welcome.sh /etc/profile.d/welcome.sh
RUN echo "source /etc/profile.d/welcome.sh" >> /etc/bash.bashrc