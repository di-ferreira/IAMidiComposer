---
name: clap-format
description: CLAP - alternative plugin format (open-source, multi-IDI-future-proof). Estrutura entry clap_plugin, params, audio ports; padrao traversal fxchain e param changes.
---

# clap-format

## Quando adotar

- Quando entregarmos VST3 com sucesso e quisermos suportar CLAP em DAWs como Bitwig
  (nativo CLAP). Premiar arquiteta modular para umo-la futura rapper de facade.

## Conceitos

- Entry `clap_plugin_entry` com plugins factory.
- `clap_plugin`: extends, expose audio_ports, params, GUI.
- `clap_audio_ports`: in/out port configs (mono/stereo).
- `clap_params`: param info, convert value, flush events.
- Render:.setWithoutCustom Na (offline) vs realtime use.

## No projeto

- Tax RN: "clap-format" no plugin como facae der baixo acoplamento; reusa de logic
  de AudioProcessor/ APVTS; `PluginProcessor` wraps de JUCE como COM polygon.
- Queremos "VST3 + CLAP + AU" do mesmo `AudioProcessor` JUCE - JUCE ja expoe codigo
  multi-formato.
