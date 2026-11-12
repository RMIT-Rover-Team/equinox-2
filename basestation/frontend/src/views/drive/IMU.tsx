import { useState, useEffect } from 'react'
import styles from './IMU.module.scss'

import { useTopic } from '../../socket/useTopic';
import { type ReceiveIMU } from '../../socket/data/imu';

function IMU() {
  const [roll, setRoll] = useState<number>(0);
  const [pitch, setPitch] = useState<number>(0);
  const [yaw, setYaw] = useState<number>(0);

  const imu = useTopic<ReceiveIMU>('IMU');
  useEffect(() => {
  	const start = performance.now();
  	const id = setInterval(() => {
      const t = (performance.now() - start) / 1000;
  		setRoll(70 * Math.sin(t));
      setPitch(80 * Math.sin(t * 0.7));
      setYaw(180 * Math.sin(t))
  	}, 30);
  	return () => clearInterval(id);
  }, []);

  return (
    <div id="IMU">
      {/*placeholder IMU data*/}
      <h1>{imu ? `${imu.pitch.toFixed(1)}°` : '--'}</h1>
      {/*move the shitty div styles later i just couldn't be bothered in the moment */}
      <img className={styles.compass} src="troll_face.png" width='100px' style={{'--yaw': `${yaw}deg`} as React.CSSProperties} />
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
