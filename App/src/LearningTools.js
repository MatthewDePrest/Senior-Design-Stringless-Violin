import React, { useState, useRef, useEffect } from "react";
import VexFlow from "vexflow";

function LearningTools({ onBack }) {
    const [input, setInput] = useState("");
    const [notes, setNotes] = useState([]);
    const notationRef = useRef(null);

    const { Renderer, Stave, StaveNote, Accidental, Formatter } = VexFlow;

    useEffect(() => {
        const div = notationRef.current;
        if (!div) return;
        div.innerHTML = "";

        const renderer = new Renderer(div, Renderer.Backends.SVG);

        const contextWidth = 1000;
        const staveWidth = 950;
        const lineSpacing = 120;

        const NOTES_PER_LINE = 12;
        const NOTES_PER_MEASURE = 4;

        const numLines = Math.ceil(notes.length / NOTES_PER_LINE);
        const totalHeight = 40 + numLines * lineSpacing + 80;
        renderer.resize(contextWidth, totalHeight);
        const context = renderer.getContext();

        const lineGroups = [];
        for (let i = 0; i < notes.length; i += NOTES_PER_LINE) {
            lineGroups.push(notes.slice(i, i + NOTES_PER_LINE));
        }

        lineGroups.forEach((group, lineIndex) => {
            const y = 40 + lineIndex * lineSpacing;

            const measureGroups = [];
            for (let i = 0; i < group.length; i += NOTES_PER_MEASURE) {
                measureGroups.push(group.slice(i, i + NOTES_PER_MEASURE));
            }

            var measureWidth = staveWidth / measureGroups.length;

            measureGroups.forEach((measure, measureIndex) => {
                var xStart = 10 + measureWidth * measureIndex;
                const measureStave = new Stave(xStart, y, measureWidth);
                measureStave.setContext(context);

                const isLastMeasure = (lineIndex === lineGroups.length - 1) && (measureIndex === measureGroups.length - 1);
                if (isLastMeasure) {
                    measureStave.setEndBarType(VexFlow.Barline.type.END);
                } else {
                    measureStave.setEndBarType(VexFlow.Barline.type.SINGLE);
                }

                if (lineIndex === 0 && measureIndex === 0) {
                    measureStave.addClef("treble").addTimeSignature(`${NOTES_PER_MEASURE}/4`);
                } else if (measureIndex === 0) {
                    measureStave.addClef("treble");
                }
                measureStave.draw();

                const staveNotes = measure.map((n) => {
                    const note = new StaveNote({
                        clef: "treble",
                        // keys: [`${n.letter.toLowerCase()}${n.sharp ? "#" : ""}/${n.octave}`], // <--- don't think this is necessary since the # is added later
                        keys: [`${n.letter.toLowerCase()}/${n.octave}`], // append the second note to this so that the expected and actual are both displayed. (or replace completely)
                        duration: "q",
                    });
                    if (n.sharp) note.addModifier(new Accidental("#"), 0);
                    // note.setStyle({ fillStyle: "green", strokeStyle: "green" }); //   <--- use this to color the "correct" or "wrong" notes
                    return note;
                });

                Formatter.FormatAndDraw(context, measureStave, staveNotes);
            });
        });
    }, [notes]);


    const handleAddNote = () => {
        const raw = input.trim().toUpperCase();
        const match = raw.match(/^([A-G])(#?)(\d)$/);

        if (!match) {
            alert("Use format like C4, F#5, or A3");
            return;
        }

        const [, letter, sharp, octave] = match;
        setNotes((prev) => [...prev, { letter, sharp: sharp === "#", octave }]);
        setInput("");
    };

    const handleClear = () => setNotes([]);

    const loadPreset = async (file) => {
        try {
            const res = await fetch(`/presets/${file}`);
            if (!res.ok) throw new Error("Failed to load preset");
            const text = await res.text();
            const tokens = text.toUpperCase().split(/[\s,]+/).filter(Boolean);

            const parsedNotes = []
            for (const token of tokens) {
                const match = token.match(/^([A-G])(#?)(\d)$/);
                if (match) {
                    const [, letter, sharp, octave] = match;
                    parsedNotes.push({ letter, sharp: sharp === "#", octave });
                }
            }
            if (parsedNotes.length === 0) {
                alert("No notes found in preset");
                return;
            }
            setNotes(parsedNotes);
        } catch (err) {
            console.error(err);
            alert("Error loading preset");
        }
    }

    return (
        <div style={{ textAlign: "center", marginTop: "40px" }}>
        <button onClick={onBack}>Back</button>
        <h2>Learning Tools</h2>

        <div style={{ marginBottom: "20px" }}>
            <button onClick={() => loadPreset("preset1.txt")}>Preset 1</button>
            <button onClick={() => loadPreset("preset2.txt")} style={{ marginLeft: "10px" }}>Preset 2</button>
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
