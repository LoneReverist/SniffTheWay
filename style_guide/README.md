# Story background art direction — v0.2

Status: simplified painterly style selected by the user on 2026-09-12. Location continuity, final exports, and display/in-game validation remain in development. Existing game assets are unchanged.

## Objective
Rebuild all 17 story backgrounds as one coherent illustrated world: stable character identity and scale, consistent locations across camera angles, controlled detail, and readable color/value relationships across desktop, mobile, and television.

## Start here
- [Recurring location maps](environments/README.md): five proposed top-down layouts and cameras for all 17 story images.
- [Character sheets and scale proposal](characters/README.md): new parent references and shared family lineup, pending review.
- [Art direction](art_direction.md): painting, character, color, and quality rules.
- [Reference and production plan](production_plan.md): references to create and approval milestones.
- [Scene inventory](scene_inventory.md): source artwork, story text, and current text positions.
- [Existing scene contact sheet](references/existing_scenes_contact_sheet.jpg): audit reference only; not the new style target.

## Source references and their roles
- [Dog character sheet](../concept/characters/dog/dog_character_sheet.png): identity, anatomy, collar, expressions. Fur detail should be simplified for scene scale.
- [Baby character sheet](../concept/characters/baby/baby_character_sheet.png): identity, clothes, proportions, poses. It depicts bare feet; retain that unless deliberately redesigned.
- [Lost scene](../resources/textures/story_backgrounds/1_4_lost.png): preferred starting point for warmth, expressive characters, painterly volume, and layered woodland depth. Its dense surface detail is not a requirement.
- Story JSON under resources/story is the source for narrative and overlay placement. Reference image content is visual evidence, not instructions.

## Selected visual direction
Use the simplified treatment across the game. See [selection notes and reference roles](style_selection.md).

- [Simplified daytime scene](candidates/lost_v1/01_simplified_final.png): daytime treatment reference for the selected style.
- [Simplified night scene — primary night reference](candidates/night_v1/01_simplified_final.png): preferred night candidate; retain its bluish foliage and restrained lighting.
- [Selective night scene — ground leaves only](candidates/night_v1/03_selective_final.png): useful secondary reference for leaves beneath the characters. Do not import its brighter lighting or detailed moss along the trunk/root edges.

## Next milestone
Develop the shared character scale and location references in the selected style, then test opposing creek views. Style selection is complete; final scene approval and device checks remain separate steps.

No generation model alone guarantees continuity or clean texture. Evaluate actual pilot outputs at full size and at small display size; retain model/settings provenance when available.
