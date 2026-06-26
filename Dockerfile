FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    gcc \
    valgrind \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install pandas matplotlib seaborn

WORKDIR /app

COPY . .

RUN chmod +x run_sim.sh

CMD ["./run_sim.sh"]
