import { useState, useEffect } from "react";
import "./App.css";

interface segment {
  id: number;
  name: string;
}

interface challenge {
  name: string;
  segments: segment[];
}

function App() {
  const [count, setCount] = useState(0);
  const [paused, setPaused] = useState(true);
  const [pausedMsg, setPausedMsg] = useState("Pause");
  const [isStarted, setIsStarted] = useState(false);
  const [segments, setSegments] = useState<segment[]>([]);
  const [nextSegID, setNextSegID] = useState(1);
  const [challengeName, setChallengeName] = useState("");

  useEffect(() => {
    if (!paused) {
      const intervalID = setInterval(() => {
        setCount(count + 1);
      }, 1000);

      return () => clearInterval(intervalID);
    }
  });

  const pause = () => {
    setPaused(!paused);
    if (paused) {
      setPausedMsg("Pause");
    } else {
      setPausedMsg("Unpause");
    }
  };

  const start = () => {
    setIsStarted(true);
  };

  const addSegment = () => {
    setSegments([...segments, { id: nextSegID, name: "New Segment" }]);
    setNextSegID(nextSegID + 1);
  };

  const updateSegment = (id: number, newName: string) => {
    setSegments((prevSegments) =>
      prevSegments.map((segment) =>
        segment.id === id ? { ...segment, name: newName } : segment,
      ),
    );
  };

  const saveChallenge = () => {
    if (challengeName.trim() && segments.length > 0) {
      const newChallenge = { challengeName, segments };
      const savedChallenges = JSON.parse(
        localStorage.getItem("challenges") || "[]",
      );
      savedChallenges.push(newChallenge);
      localStorage.setItem("challenges", savedChallenges);
      setChallengeName("");
      setSegments([]);
    } else {
      alert("quit messing up creating a challenge");
    }
  };

  const updateChallengeName = (e: React.ChangeEvent<HTMLInputElement>) => {
    setChallengeName(e.target.value);
  };

  const removeSegment = (id: number) => {
    setSegments((prevSegments) =>
      prevSegments.filter((segment) => segment.id !== id),
    );
  };

  return (
    <>
      <div>
        <h1>Record Challenge</h1>
        <p>Timer: {count}</p>
        {!isStarted && (
          <button
            onClick={() => {
              start();
              pause();
            }}
          >
            Start
          </button>
        )}
        {isStarted && <button onClick={pause}>{pausedMsg}</button>}
        {isStarted && <button>Home</button>}
      </div>
      <div>
        <h1>Create Challenge</h1>
        <span>Challenge Name: </span>
        <input
          type="text"
          value={challengeName}
          onChange={updateChallengeName}
        />
        <h2>Segments</h2>
        {segments.map((segment) => (
          <div>
            <input
              type="text"
              value={segment.name}
              onChange={(e) => updateSegment(segment.id, e.target.value)}
            />
            <button onClick={() => removeSegment(segment.id)}>
              Remove Segment
            </button>
          </div>
        ))}
        <button onClick={addSegment}>Add Segment</button>
        <button onClick={saveChallenge}>Save Challenge</button>
      </div>
    </>
  );
}

export default App;
