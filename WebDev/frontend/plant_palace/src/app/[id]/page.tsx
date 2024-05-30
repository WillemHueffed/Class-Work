import styles from "./page.module.css";

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

function PlantDetailsComponent({ params }: PageParams) {
  return <> hello world </>;
}

export default PlantDetailsComponent;
