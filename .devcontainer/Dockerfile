ARG DOCKER_TAG=latest
FROM espressif/idf:${DOCKER_TAG}

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

# Update and install dependencies
RUN apt-get update -y && \
  apt-get install -y curl udev

# Install Node.js
RUN curl -fsSL https://deb.nodesource.com/setup_23.x | bash - && \
  apt-get install -y nodejs

# Install pnpm
RUN npm install -g pnpm

# Show versions
RUN node -v && npm -v && pnpm -v

RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]

CMD ["/bin/bash", "-c"]