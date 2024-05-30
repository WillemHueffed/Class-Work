'use client';

import React, { useState } from 'react';

interface Plant {
    id: number;
    name: string;
    species: string;
    stage: string;
    location: string;
    cultivar?: string;
}

interface PlantListProps {
    plants: Plant[];
}

const PlantList: React.FC<PlantListProps> = ({ plants }) => {
    const [expandedLocations, setExpandedLocations] = useState<{ [key: string]: boolean }>({});

    const toggleLocation = (location: string) => {
        setExpandedLocations((prev: { [key: string]: boolean }) => ({
            ...prev,
            [location]: !prev[location],
        }));
    };

    const groupByLocation = (data: Plant[]) => {
        return data.reduce<{ [key: string]: Plant[] }>((acc: { [key: string]: Plant[] }, plant: Plant) => {
            const location = plant.location;
            if (!acc[location]) {
                acc[location] = [];
            }
            acc[location].push(plant);
            return acc;
        }, {});
    };

    const plantsByLocation = groupByLocation(plants);

    return (
        <div className="App">
            <h1>Plant Tracker</h1>
            {Object.keys(plantsByLocation).map((location) => (
                <div key={location}>
                    <h2 onClick={() => toggleLocation(location)} style={{ cursor: 'pointer' }}>
                        {location}
                    </h2>
                    {expandedLocations[location] && (
                        <ul>
                            {plantsByLocation[location].map((plant) => (
                                <li key={plant.id}>
                                    <h3>{plant.name}</h3>
                                    <p>Species: {plant.species}</p>
                                    <p>Stage: {plant.stage}</p>
                                    {plant.cultivar && <p>Cultivar: {plant.cultivar}</p>}
                                </li>
                            ))}
                        </ul>
                    )}
                </div>
            ))}
        </div>
    );
};

export default PlantList;

