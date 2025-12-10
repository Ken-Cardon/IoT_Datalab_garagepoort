# Python runtime als basisimage
FROM python:3.11-slim-bookworm

# Installeer systeempakketten die nodig zijn voor sensorinteractie
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    libgpiod-dev \
    build-essential && \
    rm -rf /var/lib/apt/lists/*

# Gebruik een herkenbare working directory
WORKDIR /src/app

# Kopieer de afhankelijkheden en installeer de Python-packages
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Kopieer je Python script naar de container
COPY garage.py .

# Stel het script in als startpunt
CMD ["python", "./garage.py"]
