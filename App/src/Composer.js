import React, { useState, useRef, useEffect } from "react";
import VexFlow from "vexflow";

function ComposerTool({ onBack }) {  // Add `onBack` as a prop
    const [notes, setNotes] = useState([]);  // Array of notes to be displayed
    const [timeSignature, setTimeSignature] = useState("4/4");  // Time signature toggle
    const notationRef = useRef(null);  // Ref to hold the notation container for rendering

    const { Renderer, Stave, StaveNote, Accidental, Formatter, Barline } = VexFlow;

    const NOTES_PER_LINE = 12;
    const NOTES_PER_MEASURE = timeSignature === "4/4" ? 4 : 3;  // Adjust number of notes per measure based on time signature
    const LINE_SPACING = 120;
    const STAVE_WIDTH = 950;
    const CONTEXT_WIDTH = 1000;

    const QUARTER_NOTE_MS = 500;
    const liveNotesInterval = useRef(null);

    // Fetch live notes from the server
    const fetchLiveNotes = async () => {
        try {
            const res = await fetch("http://localhost:5000/live/notes.txt");
            if (!res.ok) return;

            const text = await res.text();
            const tokens = text.toUpperCase().split(/[\s,]+/).filter(Boolean);

            // Set the notes state based purely on the contents of notes.txt
            setNotes(() => {
                const newNotes = [];
                for (const token of tokens) {
                    const match = token.match(/^([A-G])(#?)(\d)$/);
                    if (match) {
                        const [, letter, sharp, octave] = match;
                        newNotes.push({ letter, sharp: sharp === "#", octave });
                    }
                }
                return newNotes;
            });
        } catch (err) {
            console.error("Failed to load live notes", err);
        }
    };

    // Poll for live notes every 500ms
    const startPolling = () => {
        if (liveNotesInterval.current) {
            clearInterval(liveNotesInterval.current);
        }
        liveNotesInterval.current = setInterval(fetchLiveNotes, QUARTER_NOTE_MS);
    };

    // Stop polling live notes when the component unmounts
    useEffect(() => {
        startPolling();  // Start polling when the component mounts

        return () => {
            if (liveNotesInterval.current) {
                clearInterval(liveNotesInterval.current);  // Cleanup polling on unmount
            }
        };
    }, []);

    // Draw the notes on the staff
    useEffect(() => {
        const div = notationRef.current;
        if (!div) return;

        // Clear previous notation
        div.innerHTML = "";

        const renderer = new Renderer(div, Renderer.Backends.SVG);
        const numLines = Math.ceil(notes.length / NOTES_PER_LINE);
        const totalHeight = 40 + numLines * LINE_SPACING + 80;
        renderer.resize(CONTEXT_WIDTH, totalHeight);
        const context = renderer.getContext();

        const lineGroups = [];
        for (let i = 0; i < notes.length; i += NOTES_PER_LINE) {
            lineGroups.push(notes.slice(i, i + NOTES_PER_LINE));
        }

        lineGroups.forEach((group, lineIndex) => {
            const y = 40 + lineIndex * LINE_SPACING;
            const measureGroups = [];
            for (let i = 0; i < group.length; i += NOTES_PER_MEASURE) {
                measureGroups.push(group.slice(i, i + NOTES_PER_MEASURE));
            }

            const expectedMeasuresPerLine = NOTES_PER_LINE / NOTES_PER_MEASURE;
            const measureWidth = STAVE_WIDTH / expectedMeasuresPerLine;

            measureGroups.forEach((measure, measureIndex) => {
                const xStart = 10 + measureWidth * measureIndex;
                const stave = new Stave(xStart, y, measureWidth);
                stave.setContext(context);
                stave.setEndBarType(Barline.type.SINGLE);

                if (lineIndex === 0 && measureIndex === 0) {
                    stave.addClef("treble").addTimeSignature(timeSignature);  // Add the chosen time signature
                } else if (measureIndex === 0) {
                    stave.addClef("treble");
                }

                stave.draw();

                const staveNotes = measure.map((n) => {
                    const note = new StaveNote({
                        clef: "treble",
                        keys: [`${n.letter.toLowerCase()}/${n.octave}`],
                        duration: "q",  // All notes are quarter notes
                    });

                    if (n.sharp) note.addModifier(new Accidental("#"), 0);

                    return note;
                });

                Formatter.FormatAndDraw(context, stave, staveNotes);
            });
        });
    }, [notes, timeSignature]);

    // Handle the clear button action (Clear app state and notes.txt)
    const handleClear = async () => {
        setNotes([]);  // Clear notes in the app
        try {
            await fetch("http://localhost:5000/live/notes.txt", {
                method: "DELETE",  // Clear the file
            });
            console.log("notes.txt cleared");
        } catch (err) {
            console.error("Failed to clear notes.txt", err);
        }
    };

    // // Function to download the SVG as PNG
    // const downloadSheetMusic = () => {
    //     // Find the container that holds the SVG
    //     const sheetMusicElement = notationRef.current.querySelector("svg");

    //     // Ensure the SVG element exists
    //     if (!sheetMusicElement) {
    //         alert("No SVG element found.");
    //         return;
    //     }

    //     // Convert SVG to a string (the raw SVG markup)
    //     const svgString = new XMLSerializer().serializeToString(sheetMusicElement);

    //     // Create a Blob from the SVG string
    //     const svgBlob = new Blob([svgString], { type: "image/svg+xml" });

    //     // Create a temporary download link for the SVG file
    //     const link = document.createElement("a");
    //     link.href = URL.createObjectURL(svgBlob);  // Convert Blob to a URL
    //     link.download = "sheet_music.svg";  // Set the file name for the download

    //     // Trigger the download
    //     link.click();
    // };

    return (
        <div style={{ textAlign: "center", marginTop: "40px" }}>
            <button onClick={onBack}>Back</button>  {/* Back Button Added */}
            
            <h2>Composer Tool</h2>

            <div style={{ marginBottom: "20px" }}>
                <button onClick={() => setTimeSignature(timeSignature === "4/4" ? "3/4" : "4/4")}>
                    Toggle Time Signature
                </button>
            </div>

            <div style={{ marginTop: "20px" }}>
                <button onClick={handleClear}>Clear</button>
            </div>

            {/* Download Button */}
            {/* <div style={{ marginTop: "20px" }}>
                <button onClick={downloadSheetMusic}>Download Sheet Music</button>
            </div> */}

            <div
                ref={notationRef}
                style={{ marginTop: "20px", display: "flex", justifyContent: "center" }}
            ></div>
        </div>
    );
}

export default ComposerTool;
