# Не просто создаём образ, но даём ему имя build
FROM gcc:13.2 AS build

	
	RUN apt update &&\
	apt install -y python3-pip cmake && \
    pip3 install conan==1.64.1 --break-system-packages &&\
	apt install --only-upgrade libstdc++6
	
	RUN apt-get install binutils -y &&\
		apt install software-properties-common -y 
	RUN apt install python3-launchpadlib -y
		
	RUN	add-apt-repository ppa:ubuntu-toolchain-r/test &&\
		apt-get update && \
		apt-get install --only-upgrade libstdc++6 -y
	
# Запуск conan как раньше
COPY conanfile.txt /app/
RUN mkdir /app/build && cd /app/build && \
    conan install .. --build=missing


# COPY ./src /app/src
# COPY ./data/config.json /app/data/config.json 
COPY main.cpp /app/
COPY my_logger.h /app/
COPY CMakeLists.txt /app/

RUN cd /app/build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .

RUN	apt install -y rsyslog 
CMD reboot
# RUN usermod -a -G syslog $USER 

