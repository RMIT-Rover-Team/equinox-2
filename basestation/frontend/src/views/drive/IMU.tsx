import { useState, useEffect } from "react";
import styles from "./IMU.module.scss";
import { ArtificialHorizon } from "../../components/artificialHorizon/artificialHorizon";

function IMU() {
  const [roll, setRoll] = useState<number>(0);
  const [pitch, setPitch] = useState<number>(0);
  const [yaw, setYaw] = useState<number>(0);

  useEffect(() => {
    const start = performance.now();
    const id = setInterval(() => {
      const t = (performance.now() - start) / 1000;
      setRoll(70 * Math.sin(t));
      setPitch(80 * Math.sin(t * 0.7));
      setYaw(180 * Math.sin(t));
    }, 30);
    return () => clearInterval(id);
  }, []);

  return (
    <div id="IMU">
      <div className={styles.artificialHorizon}>
        <ArtificialHorizon
          pitch={pitch}
          roll={roll}
        />
      </div>
    </div>
  );
}

export default IMU;
