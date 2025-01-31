# Не просто создаём образ, но даём ему имя build
FROM gcc:13.2 AS build
	ARG DEBIAN_FRONTEND=noninteractive
	RUN echo "Europe/Moscow" > /etc/timezone
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
    conan install .. --build=missing -s compiler.libcxx=libstdc++11 -s build_type=Debug
RUN apt install libc6 -y	
COPY ./sql /app/sql
COPY ./src /app/src
COPY ./static /app/static
COPY ./tests /app/tests
COPY ./data/config.json /app/data/config.json 
COPY CMakeLists.txt /app/

RUN cd /app/build && \
    cmake -DCMAKE_BUILD_TYPE=Debug .. && \
    cmake --build .



# Второй контейнер в том же докерфайле
FROM ubuntu:22.04 AS run
	ARG DEBIAN_FRONTEND=noninteractive
	RUN echo "Europe/Moscow" > /etc/timezone
	RUN apt update &&\
	apt install -y python3-pip cmake
	
	RUN apt install --only-upgrade libstdc++6 -y &&\ 
		apt-get install binutils -y &&\
		apt install software-properties-common -y &&\
		apt install python3-launchpadlib -y &&\
		add-apt-repository ppa:ubuntu-toolchain-r/test &&\
		apt update -y && \
		apt-get install --only-upgrade libstdc++6 -y
	RUN pip install -U pytest
	RUN apt install libc6 -y

# Скопируем приложение со сборочного контейнера в директорию /app.
# Не забываем также папку data, она пригодится.
COPY --from=build /app/build/bin/game_server /app/
COPY ./data /app/data
COPY ./tests /app/tests
COPY ./static /app/static
COPY ./sql /app/sql
COPY ./src /app/src

ENV GAME_DB_URL=postgres://postgres:Mys3Cr3t@172.17.0.2:5432/postgres
# ENV GAME_DB_URL=postgres://postgres:Mys3Cr3t@postgres:5432/postgres
# COPY --from=build app/data/config.json /app/data/config.json

# Создадим пользователя www
# RUN groupadd -r www && useradd -r -g www www

RUN chmod 777 /app/data/config.json

# переключение на пользователя
#USER www


EXPOSE 8080

# Запускаем игровой сервер /app/game_server -c/app/data/config.json -w/app/static/ --state-file=save.txt --save-state-period=1000 --tick-period=50
ENTRYPOINT ["/app/game_server", "-c/app/data/config.json", "-w/app/static/","--state-file=save.txt","--save-state-period=1000"]
