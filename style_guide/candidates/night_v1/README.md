# Night scene candidates v1

> Selection update, 2026-09-12: simplified style selected for the game. See [current reference roles](../../style_selection.md). The generation-time review below records the initial candidate status; final game-asset validation remains pending.


Built-in image_gen, two stages each: background generation then character insertion. Original game artwork unchanged. Draft candidates; no display-device or in-game validation yet.

The matching lost_v1 backgrounds served as painting references, with original 3_3_night.png defining composition. Character sheets define identity and costume; the original night image defines sleeping/watchful poses. Bare feet replace the original scene's shoes to maintain sheet continuity.

## Visual review

Simplified uses the broadest leaf and fern shapes. Moderate retains richer bark and ground detail. Selective softens background and peripheral forms. All retain sleeping baby beside alert dog. Fur detail remains fairly high; geometry and character placement vary slightly between candidates. Background preservation during insertion is visual, not pixel-exact. None is an approved style master.

## 01_simplified

[Background](01_simplified_background.png) · [Complete scene](01_simplified_final.png)

### Background prompt

Use case: style-transfer. Single full-frame 16:9 storybook illustration, no panels or text. Image 1 defines the NIGHT SCENE COMPOSITION: enormous tree trunk filling left, arched root shelter at lower left-center, ivy, moss, ferns, purple flowers at lower right, winding forest path receding right, tall forest silhouettes and full moon upper right, sparse fireflies. REMOVE baby and dog completely and paint the empty sheltered ground beneath them. Image 2 defines PAINTING TREATMENT ONLY; translate it into nighttime, do not import its daylight or geography. Preserve the night scene camera, root arch, moon and major landmark positions. Cool restrained blue/teal night atmosphere, softly warm shelter earth, gentle readable midtone separation, rich shadows without black clipping. Keep nighttime convincing, not daylight tinted blue. Clean painted shapes, no grain, stippling, dense pinpoint highlights, micro-speckled bark or grass. Fireflies are a few deliberate luminous accents, not pervasive flecks. Leave lower-left/center sheltered area empty for characters. SIMPLIFIED treatment: large interlocking painted foliage masses, broad smooth bark planes with only a few grooves, grouped fern silhouettes, sparse flowers, quiet shelter floor. Strong simplification yet dimensional storybook painting, not flat vector.

## 02_moderate

[Background](02_moderate_background.png) · [Complete scene](02_moderate_final.png)

### Background prompt

Use case: style-transfer. Single full-frame 16:9 storybook illustration, no panels or text. Image 1 defines the NIGHT SCENE COMPOSITION: enormous tree trunk filling left, arched root shelter at lower left-center, ivy, moss, ferns, purple flowers at lower right, winding forest path receding right, tall forest silhouettes and full moon upper right, sparse fireflies. REMOVE baby and dog completely and paint the empty sheltered ground beneath them. Image 2 defines PAINTING TREATMENT ONLY; translate it into nighttime, do not import its daylight or geography. Preserve the night scene camera, root arch, moon and major landmark positions. Cool restrained blue/teal night atmosphere, softly warm shelter earth, gentle readable midtone separation, rich shadows without black clipping. Keep nighttime convincing, not daylight tinted blue. Clean painted shapes, no grain, stippling, dense pinpoint highlights, micro-speckled bark or grass. Fireflies are a few deliberate luminous accents, not pervasive flecks. Leave lower-left/center sheltered area empty for characters. MODERATE storybook treatment: dimensional painted leaf clusters with selected individual leaves, restrained bark grooves, legible fern fronds and grouped flowers. Rich natural painted volume like image 2, cleaner than the original night image, never uniformly sharp or granular.

## 03_selective

[Background](03_selective_background.png) · [Complete scene](03_selective_final.png)

### Background prompt

Use case: style-transfer. Single full-frame 16:9 storybook illustration, no panels or text. Image 1 defines the NIGHT SCENE COMPOSITION: enormous tree trunk filling left, arched root shelter at lower left-center, ivy, moss, ferns, purple flowers at lower right, winding forest path receding right, tall forest silhouettes and full moon upper right, sparse fireflies. REMOVE baby and dog completely and paint the empty sheltered ground beneath them. Image 2 defines PAINTING TREATMENT ONLY; translate it into nighttime, do not import its daylight or geography. Preserve the night scene camera, root arch, moon and major landmark positions. Cool restrained blue/teal night atmosphere, softly warm shelter earth, gentle readable midtone separation, rich shadows without black clipping. Keep nighttime convincing, not daylight tinted blue. Clean painted shapes, no grain, stippling, dense pinpoint highlights, micro-speckled bark or grass. Fireflies are a few deliberate luminous accents, not pervasive flecks. Leave lower-left/center sheltered area empty for characters. SELECTIVE DETAIL treatment: carefully chosen defined edges around root shelter and foreground flowers, broad soft distant forest silhouettes and subdued ivy masses. The empty shelter is the future character focal area. Background softness is painted atmospheric simplification, not lens blur. Keep textures restrained and emphasize large readable forms.

## Character insertion prompt

Use case: compositing. Image 1 is the edit target, the completed NIGHT BACKGROUND: preserve its colors, layout, lighting, painting treatment and all environment geometry. Images 2 and 3 are dog and baby character sheets: use these for identity, anatomy and clothes. Image 4 is the original night illustration: use ONLY for character poses, expressions, relative scale and placement. Add exactly one baby and one dog under the arched tree root. Match original: baby curled sleeping on his side at lower left, knees tucked, eyes peacefully closed, head resting on folded hands against dog's left flank; dog lying on belly immediately right, front paws extended, head raised looking slightly up toward screen-right, eyes open alert and protective, mouth closed. Approximate image bounds: baby x19–44%, y45–78%; dog x34–61%, y40–77%. Baby short brown tufted hair, blue short-sleeve shirt, gray trousers, BARE FEET as character sheet, no shoes or socks. Dog cream/apricot grouped curls, golden floppy ears, brown eyes, black nose, blue collar with round gold tag. Preserve sheet faces/proportions while matching image 1's painterly simplification, broad fur clumps not hundreds of tiny curls. Soft warm reflected light on faces/fur, cool blue moonlit edge accents, natural contact shadows, readable faces without looking spotlit. Preserve background outside subjects and contact shadows. No additional subjects, text, panels, noise or micro-speckles. One full-frame landscape illustration.

