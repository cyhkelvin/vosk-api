#!/usr/bin/env python3

from vosk import Model, BatchRecognizer, GpuInit, GpuThreadInit
import sys
import os
import wave

GpuInit()
GpuThreadInit()

model = Model("model")
rec = BatchRecognizer(model, 16000.0)

fnames = open("tedlium.list").readlines()
fds = [open(x.strip(), "rb") for x in fnames]
ended = set()
while True:
    for i, fd in enumerate(fds):
        if i in ended:
            continue
        data = fd.read(16000)
        if len(data) == 0:
            rec.FinishStream(i)
            ended.add(i)
        else:
            rec.AcceptWaveform(i, data)
    rec.Results()
    if len(ended) == len(fds):
        break
