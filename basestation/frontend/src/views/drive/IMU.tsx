import { useState, useEffect } from 'react'
import styles from './IMU.module.scss'

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
  	}, 30);
  	return () => clearInterval(id);
  }, []);

  return (
    <div id="IMU">
      <svg>
      <circle cx="50" cy="50" r="40" stroke="white" stroke-width="10" rotate={yaw}></circle>
      </svg>
      <div style={{ 'overflow': 'hidden', 'width': '80px', 'height': '80' }}>
        <img src="troll_face.png"
          className={styles.horizon}
          style={
            {
              '--roll': `${roll}deg`,
              '--pitch': pitch
            } as React.CSSProperties}
            width='100px' />
      </div>
    </div>
  );
}

export default IMU
