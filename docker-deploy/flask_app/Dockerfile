FROM python:3.8-slim

WORKDIR /app

RUN pip install Flask

COPY . /app

EXPOSE 5000

CMD ["flask", "run", "--host=0.0.0.0"]
