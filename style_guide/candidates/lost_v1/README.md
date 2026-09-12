# Lost scene — first style candidates

> Selection update, 2026-09-12: simplified style selected for the game. See [current reference roles](../../style_selection.md). The generation-time review below records the initial candidate status; final game-asset validation remains pending.


Generated using the built-in image_gen tool in two stages per variation: empty background, then character insertion using the supplied identity sheets and original pose/expression reference. Original game asset unchanged. Outputs are draft candidates, not approved style masters.

## Review
1. Simplified: broadest foliage shapes and clearest simplification.
2. Moderate: closest to the original richness; more small foliage marks remain.
3. Selective: softer distant forest, with detailed foreground framing.

All preserve the intended standing dog / crawling worried baby staging. Character placement varies slightly between outputs. Fine grass highlights and fur marks remain, especially in 2 and 3; these candidates are not a complete elimination of microdetail. Background preservation through insertion is visual, not pixel-exact.

## 01_simplified

[Background](01_simplified_background.png) · [With characters](01_simplified_final.png)

### Background prompt

Use case: style-transfer. Create one full-frame 16:9 landscape storybook background, no panels or text. Reference image is the current lost scene: preserve camera, framing, winding warm stone path, large leaning left trunk, right trunk, foreground rocks and right stump, flowers and butterflies, sky opening and layered forest. REMOVE dog and baby completely, rebuilding the empty path beneath them. Retain inviting warm daylight and natural restrained greens, cream stone, blue sky; no yellow wash. Repaint from scratch in a clean coherent children's storybook painting. Simplified painterly treatment: broad soft-edged interlocking foliage masses, minimal internal leaf marks, broad smooth bark planes, sparse grouped flowers, broad stone shapes, clean tranquil shadow areas. Most simplified of the three, still dimensional and beautifully painted, not flat vector art. No grain, micro-speckles, isolated pixel-like highlights, dense tiny grass strokes, photographic texture or sharpening. Preserve lower central space for later characters. Output only the background.

## 02_moderate

[Background](02_moderate_background.png) · [With characters](02_moderate_final.png)

### Background prompt

Use case: style-transfer. Create one full-frame 16:9 landscape storybook background, no panels or text. Reference image is the current lost scene: preserve camera, framing, winding warm stone path, large leaning left trunk, right trunk, foreground rocks and right stump, flowers and butterflies, sky opening and layered forest. REMOVE dog and baby completely, rebuilding the empty path beneath them. Retain inviting warm daylight and natural restrained greens, cream stone, blue sky; no yellow wash. Repaint from scratch in a clean coherent children's storybook painting. Moderate storybook detail treatment: dimensional painted foliage clusters with a few intentional leaf shapes at their edges, restrained broad bark grooves, readable grouped flowers and stone edges. Richer than a simplified painting but decisively cleaner and less busy than the source. No stippled detail. No grain, micro-speckles, isolated pixel-like highlights, dense tiny grass strokes, photographic texture or sharpening. Preserve lower central space for later characters. Output only the background.

## 03_selective

[Background](03_selective_background.png) · [With characters](03_selective_final.png)

### Background prompt

Use case: style-transfer. Create one full-frame 16:9 landscape storybook background, no panels or text. Reference image is the current lost scene: preserve camera, framing, winding warm stone path, large leaning left trunk, right trunk, foreground rocks and right stump, flowers and butterflies, sky opening and layered forest. REMOVE dog and baby completely, rebuilding the empty path beneath them. Retain inviting warm daylight and natural restrained greens, cream stone, blue sky; no yellow wash. Repaint from scratch in a clean coherent children's storybook painting. Selective-detail treatment: near foreground flowers and path stones have carefully chosen crisp painted edges, while upper and middle forest become very broad soft atmospheric masses. Reserve the strongest precision for the lower-center area where characters will later stand. Strong intentional focal hierarchy, not all-over detail. No grain, micro-speckles, isolated pixel-like highlights, dense tiny grass strokes, photographic texture or sharpening. Preserve lower central space for later characters. Output only the background.

## Character insertion prompt (same for all three)

Use case: compositing. Image 1 is the EDIT TARGET: keep this clean painted background, its geometry, colors, framing and detail treatment unchanged. Images 2 and 3 define dog and baby IDENTITY, anatomy, proportions and costume. Image 4 defines ONLY character positions, sizes, POSES and EXPRESSIONS; do not copy its environment or fine texture. Add exactly one dog and one baby to image 1. Match image 4: dog lower center-left, standing on all four paws, body three-quarter toward viewer, curled tail left, head raised and turned toward screen-left with concerned alert eyes, mouth closed. Baby immediately to its right, on hands and knees, body angled toward screen-right, head raised looking right, worried raised inner eyebrows and slightly parted mouth. Match original occupied bounds: dog about x27–47%, y47–90%; baby x45–64%, y53–88%. Dog cream/apricot fur, golden floppy ears, blue collar, round gold tag. Baby brown tufted hair, brown eyes, blue short-sleeved shirt, gray trousers, bare feet. Use character sheets to preserve their actual faces. Adapt their rendering to the background with broader grouped fur locks and softly modeled skin; crisp expressive eyes without dense tiny curls or speckling. Natural warm daylight and grounding contact shadows on path. Do not change background outside inserted characters and shadows. No extra subjects, text, panel layout, grain or sharpened microtexture. Single full landscape illustration.

Reference order: variant background; dog_character_sheet.png; baby_character_sheet.png; original 1_4_lost.png.

