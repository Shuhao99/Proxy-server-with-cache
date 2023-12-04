# 使用一个基础镜像，例如 Python 官方镜像
FROM python:3.8-slim

WORKDIR /app

RUN pip install Flask

COPY . /app

EXPOSE 5000

CMD ["flask", "run", "--host=0.0.0.0"]
