# FieldSec Training Academy

FieldSec v0.4 treats education and instrumentation as one workflow. A lesson is not a slideshow; it teaches a concept, guides safe setup, launches the corresponding real instrument where appropriate, asks the operator to interpret the observation, and connects the result to a defensive control.

## Lesson state

Each lesson can be Not started, In progress, or Complete. Progress is stored in `training.ini` in the app data directory.

## Curriculum

1. Engagement Fundamentals
2. UART: From Pins to Evidence
3. I2C: Discover the Trust Boundary
4. Wi-Fi Recon: Metadata to Hypothesis
5. GPIO: Observe Before Control
6. Credential Systems
7. Evidence to Finding
8. Full IoT Assessment

The UART, I2C, Wi-Fi and GPIO lessons have direct Practice links to their matching instrument or current observation guide.

## Instructional model

Each lesson uses the same six-part pattern:

- Objective: what skill the learner should acquire.
- Concept: the technical idea and trust boundary.
- Setup: safe physical or logical preparation.
- Practice: the real-world observation or assessment action.
- Interpret: how to avoid overstating what the data proves.
- Defend: the control or design change that addresses the validated issue.
- Checkpoint: a question the learner should be able to answer before marking the lesson complete.

This structure intentionally teaches the red-team reasoning loop: scope -> observe -> hypothesize -> validate -> document -> mitigate -> retest.
