"use client";
import { useRouter } from "next/navigation";
import Link from "next/link";
import styles from "./page.module.css";
import { useState, useEffect } from "react";

interface Harvest {
  date: string;
  amount: string;
}

async function getHarvestData(id: string): Promise<Harvest[]> {
  const uri = `https://cpsc4910sq24.s3.amazonaws.com/data/plants/${id}/harvests.json`;
  const res = await fetch(uri);
  if (!res.ok) {
    throw new Error("Failed to fetch data");
  }
  return res.json();
}

interface PageParams {
  params: {
    id: string;
  };
}

type Plant = {
  id: number;
  name: string;
  species: string;
  stage: string;
  location: string;
  cultivar?: string;
};

const jpgs_uri = "https://cpsc4910sq24.s3.amazonaws.com/images/";

function determine_photo_uri(species: string, cultivar?: string): string {
  if (species === "Arugula") {
    return jpgs_uri + "arugula.jpg";
  } else if (species === "Bell pepper") {
    return jpgs_uri + "bell-pepper.jpg";
  } else if (species === "Lettuce") {
    if (!cultivar) {
      return "";
    } else if (cultivar === "Butter") {
      return jpgs_uri + "butter-lettuce.jpg";
    } else if (cultivar === "Green leaf") {
      return jpgs_uri + "green-leaf-lettuce.jpg";
    }
    return "";
  } else if (species === "Strawberry") {
    return jpgs_uri + "strawberry.jpg";
  }
  return "";
}

function PlantDetailsComponent({ params }: PageParams) {
  const id = params.id;

  const router = useRouter();
  const [plantData, setPlantData] = useState<Plant | null>(null);
  const [plantHarvests, setPlantHarvests] = useState<Harvest[] | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    const getPlantData = async () => {
      try {
        const plant_res = await fetch(
          `https://cpsc4910sq24.s3.amazonaws.com/data/plants/${id}.json`,
        );
        if (!plant_res.ok) {
          throw new Error("Failed to fetch data");
        }
        const plant_result = await plant_res.json();
        setPlantData(plant_result);

        const harvest_res = await fetch(
          `https://cpsc4910sq24.s3.amazonaws.com/data/plants/${id}/harvests.json`,
        );
        if (!harvest_res.ok) {
          throw new Error("Failed to fetch data");
        }
        const harvest_result = await harvest_res.json();
        setPlantHarvests(harvest_result);

        setLoading(false);
      } catch (error: any) {
        setError(error.message);
        setLoading(false);
      }
    };

    getPlantData();
  }, []);

  if (!plantData || !plantHarvests) {
    return <>Error loading data</>;
  }

  const species = plantData.species;
  const cultivar = plantData.cultivar;
  const name = plantData.name;
  const stage = plantData.stage;

  console.log("harvests are");
  console.log(plantHarvests);

  return (
    <>
      <h1>{plantData.name}</h1>
      <div>
        <div>
          <img
            src={determine_photo_uri(species, cultivar)}
            className="details-image"
          />
          <br />
          Species: {species} <br />
          Stage: {stage}
        </div>
        <table>
          <tbody>
            <tr>
              <th>Date</th>
              <th>Amount</th>
            </tr>
            {plantHarvests.map((item: Harvest, index: number) => (
              <tr key={index}>
                <td>{item.date}</td>
                <td>{item.amount}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </>
  );
}

export default PlantDetailsComponent;
