import React, { useState, useEffect, useRef } from 'react';
import { useLocation, useNavigate } from 'react-router-dom';

interface Segment {
  id: number;
  name: string;
  time?: number;
}

interface Challenge {
  challengeName: string;
  segments: Segment[];
}

const Record_Challenge: React.FC = () => {
  const location = useLocation();
  const challengeData = location.state as { challenge: Challenge } | null;
  const navigate = useNavigate();

  const [isRunning, setIsRunning] = useState(false);
  const [isPaused, setIsPaused] = useState(false);
  const [isCompleted, setIsCompleted] = useState(false);
  const [time, setTime] = useState(0);
  const [segmentTimes, setSegmentTimes] = useState<Segment[]>(challengeData?.challenge.segments || []);
  const [bestSegmentTimes, setBestSegmentTimes] = useState<Segment[]>([]);

  const timerRef = useRef<number | null>(null);

  const goToHomeScreen = () => {
    navigate('/');
  };

  useEffect(() => {
    if (challengeData?.challenge.challengeName) {
      const savedBestTimes = localStorage.getItem(challengeData.challenge.challengeName);
      if (savedBestTimes) {
        setBestSegmentTimes(JSON.parse(savedBestTimes));
      } else {
        setBestSegmentTimes(challengeData.challenge.segments.map(segment => ({ ...segment })));
      }
    }
  }, [challengeData]);

  useEffect(() => {
    if (isRunning && !isPaused) {
      timerRef.current = window.setInterval(() => {
        setTime(prevTime => prevTime + 1);
      }, 1000);
    } else if (timerRef.current) {
      clearInterval(timerRef.current);
      timerRef.current = null;
    }

    return () => {
      if (timerRef.current) {
        clearInterval(timerRef.current);
      }
    };
  }, [isRunning, isPaused]);

  const handleStart = () => {
    setIsRunning(true);
  };

  const handleNextSegment = () => {
    const nextSegmentIndex = segmentTimes.findIndex(segment => segment.time === undefined);
    if (nextSegmentIndex !== -1) {
      const newSegmentTimes = [...segmentTimes];
      newSegmentTimes[nextSegmentIndex].time = time;
      setSegmentTimes(newSegmentTimes);
    }
  };

  const handleEnd = () => {
    setIsCompleted(true);
    setIsRunning(false);
    updateBestTimes();
  };

  const handlePauseResume = () => {
    setIsPaused(prevIsPaused => !prevIsPaused);
  };

  const updateBestTimes = () => {
    if (challengeData?.challenge.challengeName) {
      const newBestTimes = segmentTimes.map((segment, index) => {
        if (bestSegmentTimes[index].time === undefined || segment.time! < bestSegmentTimes[index].time!) {
          return { ...segment };
        } else {
          return { ...bestSegmentTimes[index] };
        }
      });
      setBestSegmentTimes(newBestTimes);
      localStorage.setItem(challengeData.challenge.challengeName, JSON.stringify(newBestTimes));
    }
  };

  const getColor = (currentTime?: number, bestTime?: number) => {
    if (currentTime === undefined) return 'black';
    if (bestTime === undefined) return 'green';
    if (currentTime < bestTime) return 'green';
    if (currentTime > bestTime) return 'red';
    return 'yellow';
  };

  return (
    <div>
      <h1>{challengeData?.challenge.challengeName}</h1>
      <div>
        <h2>Overall Timer: {time}s</h2>
      </div>
      <ul>
        {segmentTimes.map((segment, index) => (
          <li key={segment.id} style={{ color: getColor(segment.time, bestSegmentTimes[index]?.time) }}>
            {segment.name}: {segment.time !== undefined ? `${segment.time}s` : 'Not completed'}
          </li>
        ))}
      </ul>
      {!isRunning && !isCompleted && (
        <>
          <button onClick={handleStart}>Start</button>
          <button onClick={goToHomeScreen}>Home</button>
        </>
      )}
      {!isRunning && isCompleted && (
        <>
          <button onClick={goToHomeScreen}>Home</button>
        </>
      )}
      {isRunning && !isPaused && !isCompleted && (
        <>
          {segmentTimes.some(segment => segment.time === undefined) && <button onClick={handleNextSegment}>Next Segment</button>}
          <button onClick={handleEnd}>End</button>
          <button onClick={handlePauseResume}>Pause</button>
        </>
      )}
      {isRunning && isPaused && !isCompleted && (
        <button onClick={handlePauseResume}>Resume</button>
      )}
    </div>
  );
};

export default Record_Challenge;

