FROM covidwebhook:1.1.4

WORKDIR /usr/src/khobar_webhook/covidwebhook/build
CMD ["./covidwebhook"]

# COPY . .
# RUN LD_LIBRARY_PATH=/usr/local/lib && export LD_LIBRARY_PATH && cmake .
# RUN make -j4
# CMD ./covidwebhook