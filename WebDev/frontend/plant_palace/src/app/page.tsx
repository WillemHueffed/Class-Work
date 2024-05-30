"use client";
import styles from "./page.module.css";
import Link from "next/link";
import { useRouter } from "next/router";
import { useState, useEffect } from "react";

const plants_uri = "https://cpsc4910sq24.s3.amazonaws.com/data/plants.json";
const jpgs_uri = "https://cpsc4910sq24.s3.amazonaws.com/images/";

type Plant = {
  id: number;
  name: string;
  species: string;
  stage: string;
  location: string;
  cultivar?: string;
};

function sortByLocation(data: Plant[]): { [key: string]: Plant[] } {
  const locationMap: { [key: string]: Plant[] } = {};
  data.forEach((entry) => {
    const location = entry.location;
    if (!locationMap[location]) {
      locationMap[location] = [];
    }
    locationMap[location].push(entry);
  });
  return locationMap;
}

function determine_photo_uri(species: string, cultivar?: string): String {
  if (species == "Arugula") {
    return jpgs_uri + "arugula.jpg";
  } else if (species == "Bell pepper") {
    return jpgs_uri + "bell-pepper.jpg";
  } else if (species == "Lettuce") {
    if (!cultivar) {
      return "";
    } else if (cultivar == "Butter") {
      return jpgs_uri + "butter-lettuce.jpg";
    } else if (cultivar == "Green leaf") {
      return jpgs_uri + "green-leaf-lettuce.jpg";
    }
    return "";
  } else if (species == "Strawberry") {
    return jpgs_uri + "strawberry.jpg";
  }
  return "";
}

export default function Home() {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  useEffect(() => {
    const getPlantData = async () => {
      const res = await fetch(plants_uri);
      if (!res.ok) {
        throw new Error("Failed to fetch data");
      }
      const result = await res.json();
      setData(result);
      setLoading(false);
    };

    getPlantData();
  }, []);

  if (data) {
    const sortedData = sortByLocation(data);
  } else {
    throw Error;
  }
  const router = useRouter();

  return (
    <main>
      <h1>Plant Palace</h1>
      {Object.keys(sortedData).map((location) => (
        <details open key={location}>
          <summary>{location}</summary>
          <ul className="plant-list">
            {sortedData[location].map((plant) => (
              <div className="plant-entry">
                <li key={plant.id} className="plant_li">
                  <img
                    src={determine_photo_uri(plant.species, plant.cultivar)}
                    className="circle-image"
                  />
                  <div>
                    <span className={styles.plantinfo}>
                      <a href={"plant_details/1"} onClick={handleClick}>
                        {plant.name}
                      </a>
                    </span>{" "}
                    <br />
                    <div>
                      Species: {plant.species} <br />
                      {plant.cultivar && (
                        <>
                          Cultivar: {plant.cultivar} <br />
                        </>
                      )}
                      Stage: {plant.stage}
                    </div>
                  </div>
                </li>
              </div>
            ))}
          </ul>
        </details>
      ))}
    </main>
  );
}
