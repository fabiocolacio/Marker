#!/usr/bin/bash

marmite . ./site  \
  --name Marker \
  --tagline "Simple and effective Markdown Editor" \
  --enable-search true \
  --colorscheme minimal \
  --toc true \
  --image-provider picsum  \
  --watch \
  --serve \
  -vvv
