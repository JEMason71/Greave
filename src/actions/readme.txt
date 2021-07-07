The src/actions folder contains code for any and all actions the player and
NPCs can take in the game. This code should be (as much as possible) entirely
for *active* actions, such as interacting with things -- passive events, such
as the passage of time, decay, hunger, the effects of poison, etc. all belong
in the 'world' folder, as they define things that happen regardless of intent.

The 'actions/combat' folder contains combat-specific actions, since this is
the primary core of the gameplay.
