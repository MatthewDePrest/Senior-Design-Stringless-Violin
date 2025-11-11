import React, { useState, useRef, useEffect } from "react";
import VexFlow from "vexflow";

function LearningTools({ onBack }) {
    const [input, setInput] = useState("");
    const [presetNotes, setPresetNotes] = useState([]);
    const [incomingNotes, setIncomingNotes] = useState([]);
    const notationRef = useRef(null);

    const { Renderer, Stave, StaveNote, Accidental, Formatter, Barline } = VexFlow;

    const NOTES_PER_LINE = 12;
    const NOTES_PER_MEASURE = 4;
    const LINE_SPACING = 120;
    const STAVE_WIDTH = 950;
    const CONTEXT_WIDTH = 1000;

    const QUARTER_NOTE_MS = 500;

    // Merge preset and incoming notes into one array for display
    const mergedNotes = (() => {
        const maxLength = Math.max(presetNotes.length, incomingNotes.length);
        const merged = [];
        for (let i = 0; i < maxLength; i++) {
            const preset = presetNotes[i];
            const live = incomingNotes[i];

            const keys = [];
            const sharps = [];
            let isCorrect = false;

            if (preset) {
                keys.push(`${preset.letter.toLowerCase()}/${preset.octave}`);
                sharps.push(preset.sharp);
            }

            if (live) {
                // Check if live note matches preset
                if (preset &&
                    preset.letter === live.letter &&
                    preset.octave === live.octave &&
                    preset.sharp === live.sharp) {
                    isCorrect = true; // mark correct, no duplicate key
                } else {
                    keys.push(`${live.letter.toLowerCase()}/${live.octave}`);
                    sharps.push(live.sharp);
                }
            }

            merged.push({ keys, sharps, isCorrect });
        }
        return merged;
    })();

    // Draw notes whenever mergedNotes changes
    useEffect(() => {
        const div = notationRef.current;
        if (!div) return;
        div.innerHTML = "";

        const renderer = new Renderer(div, Renderer.Backends.SVG);
        const numLines = Math.ceil(mergedNotes.length / NOTES_PER_LINE);
        const totalHeight = 40 + numLines * LINE_SPACING + 80;
        renderer.resize(CONTEXT_WIDTH, totalHeight);
        const context = renderer.getContext();

        const lineGroups = [];
        for (let i = 0; i < mergedNotes.length; i += NOTES_PER_LINE) {
            lineGroups.push(mergedNotes.slice(i, i + NOTES_PER_LINE));
        }

        lineGroups.forEach((group, lineIndex) => {
            const y = 40 + lineIndex * LINE_SPACING;

            const measureGroups = [];
            for (let i = 0; i < group.length; i += NOTES_PER_MEASURE) {
                measureGroups.push(group.slice(i, i + NOTES_PER_MEASURE));
            }

            const measureWidth = STAVE_WIDTH / measureGroups.length;

            measureGroups.forEach((measure, measureIndex) => {
                const xStart = 10 + measureWidth * measureIndex;
                const stave = new Stave(xStart, y, measureWidth);
                stave.setContext(context);

                const isLastMeasure =
                    lineIndex === lineGroups.length - 1 &&
                    measureIndex === measureGroups.length - 1;

                stave.setEndBarType(isLastMeasure ? Barline.type.END : Barline.type.SINGLE);

                if (lineIndex === 0 && measureIndex === 0) {
                    stave.addClef("treble").addTimeSignature(`${NOTES_PER_MEASURE}/4`);
                } else if (measureIndex === 0) {
                    stave.addClef("treble");
                }

                stave.draw();

                const staveNotes = measure.map((n) => {
                    const note = new StaveNote({
                        clef: "treble",
                        keys: n.keys,
                        duration: "q",
                    });

                    n.sharps.forEach((s, i) => {
                        if (s) note.addModifier(new Accidental("#"), i);
                    });

                    // Determine note color
                    // if (n.isCorrect) {
                    //     note.setStyle({ fillStyle: "green", strokeStyle: "green" }); // correct
                    // } else if (n.keys.length > 1) {
                    //     // if there’s a live note that doesn’t match preset
                    //     note.setStyle({ fillStyle: "red", strokeStyle: "red" }); // wrong
                    // } else {
                    //     // note hasn’t been played yet
                    //     note.setStyle({ fillStyle: "black", strokeStyle: "black" }); // default
                    // }
                    if (n.keys.length > 1) {
                        note.setKeyStyle(0, { fillStyle: "blue", strokeStyle: "blue" });   // preset
                        note.setKeyStyle(1, { fillStyle: "red", strokeStyle: "red" });     // live (wrong)
                        note.setStemStyle({ strokeStyle: "red" });
                    } else if (n.isCorrect) {
                        note.setKeyStyle(0, { fillStyle: "green", strokeStyle: "green" }); // correct
                        note.setStemStyle({ strokeStyle: "green" });
                    } else {
                    // Default
                        note.setKeyStyle(0, { fillStyle: "black", strokeStyle: "black" }); // unplayed
                        note.setStemStyle({ strokeStyle: "black" });
                    }


                    return note;
                });

                Formatter.FormatAndDraw(context, stave, staveNotes);
            });
        });
    }, [mergedNotes]);

    // Poll /live/notes.txt every second and append new notes
    useEffect(() => {
        const interval = setInterval(async () => {
            try {
                const res = await fetch("http://localhost:5000/live/notes.txt");
                if (!res.ok) return;

                const text = await res.text();
                const tokens = text.toUpperCase().split(/[\s,]+/).filter(Boolean);

                setIncomingNotes((prev) => {
                    const newNotes = [];
                    for (const token of tokens) {
                        const match = token.match(/^([A-G])(#?)(\d)$/);
                        if (match) {
                            const [, letter, sharp, octave] = match;
                            const exists = prev.some(
                                (n) => n.letter === letter && n.sharp === (sharp === "#") && n.octave === octave
                            );
                            if (!exists) {
                                newNotes.push({ letter, sharp: sharp === "#", octave });
                            }
                        }
                    }
                    return [...prev, ...newNotes];
                });
            } catch (err) {
                console.error("Failed to load live notes", err);
            }
        }, QUARTER_NOTE_MS);

        return () => clearInterval(interval);
    }, []);

    const handleAddNote = () => {
        const raw = input.trim().toUpperCase();
        const match = raw.match(/^([A-G])(#?)(\d)$/);

        if (!match) {
            alert("Use format like C4, F#5, or A3");
            return;
        }

        const [, letter, sharp, octave] = match;
        setPresetNotes((prev) => [...prev, { letter, sharp: sharp === "#", octave }]);
        setInput("");
    };

    const handleClear = () => {
        setPresetNotes([]);
        setIncomingNotes([]);
    };

    const loadPreset = async (file) => {
        try {
            const res = await fetch(`/presets/${file}`);
            if (!res.ok) throw new Error("Failed to load preset");

            const text = await res.text();
            const tokens = text.toUpperCase().split(/[\s,]+/).filter(Boolean);

            const parsed = [];
            for (const token of tokens) {
                const match = token.match(/^([A-G])(#?)(\d)$/);
                if (match) {
                    const [, letter, sharp, octave] = match;
                    parsed.push({ letter, sharp: sharp === "#", octave });
                }
            }

            if (parsed.length === 0) {
                alert("No notes found in preset");
                return;
            }

            setPresetNotes(parsed);
        } catch (err) {
            console.error(err);
            alert("Error loading preset");
        }
    };

    return (
        <div style={{ textAlign: "center", marginTop: "40px" }}>
            <button onClick={onBack}>Back</button>
            <h2>Learning Tools</h2>

            <div style={{ marginBottom: "20px" }}>
                <button onClick={() => loadPreset("preset1.txt")}>Preset 1</button>
                <button onClick={() => loadPreset("preset2.txt")} style={{ marginLeft: "10px" }}>
                    Preset 2
                </button>
            </div>

            <p>Enter a note (A–G, optional #, then octave):</p>
            <input
                type="text"
                value={input}
                onChange={(e) => setInput(e.target.value)}
                placeholder="e.g. C4, F#5, A3"
                style={{ fontSize: "1rem", padding: "5px", marginRight: "10px" }}
            />
            <button onClick={handleAddNote}>Add Note</button>
            <button onClick={handleClear} style={{ marginLeft: "10px" }}>
                Clear
            </button>

            <div
                ref={notationRef}
                style={{ marginTop: "20px", display: "flex", justifyContent: "center" }}
            ></div>
        </div>
    );
}

export default LearningTools;
