import styles from "./page.module.css";
import Link from "next/link";

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

async function getPlantData(): Promise<Plant[]> {
    const res = await fetch(plants_uri);
    if (!res.ok) {
        throw new Error('Failed to fetch data');
    }
    return res.json();
}



function sortByLocation(data: Plant[]): { [key: string]: Plant[] } {
    const locationMap: { [key: string]: Plant[] } = {};
    data.forEach(entry => {
        const location = entry.location;
        if (!locationMap[location]) {
            locationMap[location] = [];
        }
        locationMap[location].push(entry);
    });
    return locationMap;
}

function determine_photo_uri(species: string, cultivar?: string): String{
  if (species == "Arugula"){
    return jpgs_uri + "arugula.jpg";
  }
  else if (species == "Bell pepper"){
    return jpgs_uri + "bell-pepper.jpg";
  }
  else if (species == "Lettuce"){
    if (!cultivar){
      return "";
    }
    else if (cultivar == "Butter"){
      return jpgs_uri + "butter-lettuce.jpg";
    }
    else if (cultivar == "Green leaf"){
      return jpgs_uri + "green-leaf-lettuce.jpg";
    }
    return "";
  }
  else if (species == "Strawberry"){
    return jpgs_uri + "strawberry.jpg";
  }
  return "";
}

export default async function Home() {
    const data = await getPlantData();
    const sortedData = sortByLocation(data);

    return (
        <main>
            <h1>Plant Palace</h1>
            {Object.keys(sortedData).map(location => (
                <details open key={location}>
                  <summary>{location}</summary>
                    <ul className="plant-list">
                        {sortedData[location].map(plant => (
                          <div className="plant-entry">
                            <li key={plant.id} className="plant_li">
                              <img src={determine_photo_uri(plant.species, plant.cultivar)} className="circle-image" />
                              <div>
                                <span className={styles.plantinfo}>
                                    <Link href={{pathname: "plant_details", query: {name: plant.name, species: plant.species,
                                      cultivar: plant.cultivar, stage: plant.stage,
                                      jpg: determine_photo_uri(plant.species, plant.cultivar)}}}>{plant.name}
                                    </Link>
                                  </span> <br/>
                                <div>
                                  Species: {plant.species} <br/>
                                  {plant.cultivar && (
                                    <>
                                      Cultivar: {plant.cultivar} <br/>
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


