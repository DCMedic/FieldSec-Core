# Training Academy: v0.5 Teaching Model

FieldSec teaches through the same tools used for authorized field observations.

The learning loop is:

**Concept -> Predict -> Observe -> Preserve -> Compare -> Interpret -> Defend -> Retest**

## Practice requirements

Paired lessons now record practice count. Before a paired lesson can be completed, the learner must launch the corresponding real instrument at least once. Completion still represents course progress rather than proof of professional competency.

## Workbench analysis prompts

### UART
- Is the selected baud/framing hypothesis supported by readable output?
- Which strings are direct evidence?
- Which conclusions would require interaction rather than passive observation?
- Could low printability indicate a binary protocol rather than incorrect baud?

### I2C
- Which addresses currently respond?
- What changed relative to the previous baseline?
- What benign explanations could produce the same change?
- What is the least-invasive way to identify the peripheral?

### Wi-Fi
- Which BSSIDs are new or absent compared with the prior survey?
- Does an advertised authentication mode describe the complete security posture?
- Is the network in scope and owned by the lab/client?
- Which change observations deserve additional validation?

## Finding quality

A student should be able to answer five separate questions before a finding is considered mature:

1. What condition was actually observed?
2. Where is the retained evidence?
3. What impact has actually been validated?
4. Which control addresses that condition?
5. What observable result proves remediation worked?
