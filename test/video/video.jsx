import './video.scss'

import Video from '/components/Video'

export default async () => {
  const video = Video.render({ src: 'test/30fps.mp4' })
  await video.load()

  video.play()
}
